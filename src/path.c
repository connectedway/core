/* Copyright (c) 2009 Blue Peach Solutions, Inc.
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times 
 * remain with Blue Peach Solutions.
 */
#define __BLUE_CORE_DLL__

#define AUTHENTICATE

#include "ofc/core.h"
#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/path.h"
#include "ofc/net.h"
#include "ofc/lock.h"
#include "ofc/queue.h"

#include "ofc/file.h"
#include "ofc/heap.h"
#include "ofc/persist.h"

typedef struct 
{
  BLUE_FS_TYPE type ;		/**< The Mapped File System Type   */
  BLUE_LPTSTR device ;		/**< The device name */
  BLUE_LPTSTR username ;	/**< The user name  */
  BLUE_LPTSTR password ;	/**< The password  */
  BLUE_LPTSTR domain ;		/**< The workgroup/domain  */
  BLUE_INT port ;
  BLUE_UINT num_dirs ;		/**< Number of directory fields  */
  BLUE_LPTSTR *dir ;		/**< Array of directory entries.
				   First is the share */
  BLUE_BOOL absolute ;		/**< If the path is absolute */
  BLUE_BOOL remote ;		/**< If the path is remote */
} _BLUE_PATH ;

typedef struct 
{
  BLUE_LPTSTR lpDevice ;
  BLUE_LPTSTR lpDesc ;
  _BLUE_PATH *map ;
  BLUE_BOOL thumbnail ;
} PATH_MAP_ENTRY ;

static PATH_MAP_ENTRY BluePathMaps[BLUE_PARAM_MAX_MAPS] ;
static BLUE_LOCK lockPath ;

BLUE_BOOL BluePathIsWild (BLUE_LPCTSTR dir)
{
  BLUE_LPCTSTR p ;
  BLUE_BOOL wild ;

  wild = BLUE_FALSE ;
  p = BlueCtstrtok (dir, TSTR("*?")) ;
  if (p != BLUE_NULL && *p != TCHAR_EOS)
    wild = BLUE_TRUE ;
  return (wild) ;
}

BLUE_CORE_LIB BLUE_VOID 
BluePathUpdate (BLUE_PATH *_path, BLUE_PATH *_map) 
{
  BLUE_UINT i ;

  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  _BLUE_PATH *map = (_BLUE_PATH *) _map ;

  path->type = map->type ;
  BlueHeapFree (path->device) ;
  path->device = BlueCtstrdup (map->device) ;

  if (map->username != BLUE_NULL)
    {
      BlueHeapFree (path->username) ;
      path->username = BlueCtstrdup (map->username) ;
    }

  if (map->password != BLUE_NULL)
    {
      BlueHeapFree (path->password) ;
      path->password = BlueCtstrdup (map->password) ;
    }

  if (map->domain != BLUE_NULL)
    {
      BlueHeapFree (path->domain) ;
      path->domain = BlueCtstrdup (map->domain) ;
    }

  /*
   * If there is a server in the path, we want to remove it.
   */
  if (path->remote && path->num_dirs > 0)
    {
      BlueHeapFree (path->dir[0]) ;
      for (i = 1 ; i < path->num_dirs ; i++)
	{
	  path->dir[i-1] = path->dir[i] ;
	}
      path->num_dirs-- ;
    }

  path->absolute = map->absolute ;
  path->remote = map->remote ;
  if (path->remote)
    path->port = map->port ;

  path->dir = BlueHeapRealloc (path->dir, 
			       (map->num_dirs + path->num_dirs) * 
			       sizeof (BLUE_TCHAR *)) ;
  /*
   * Now insert the mapped paths in front of the existing path dirs
   * Move paths dirs 
   */
  for (i = map->num_dirs + path->num_dirs ; i > map->num_dirs ; i -- )
    {
      path->dir[i - 1] = path->dir[i - map->num_dirs - 1] ;
    }
      
  for (i = 0 ; i < map->num_dirs ; i++)
    {
      path->dir[i] = BlueCtstrdup (map->dir[i]) ;
    }
  path->num_dirs += map->num_dirs ;
  /*
   * This is ugly.  Wish workgroups were really part of the URL
   * If the number of directories is > 1, the first directory is a workgroup
   * and the second directory is not wild, then we want to squelch the
   * workgroup
   */
  if (path->num_dirs > 1 && !BluePathIsWild(path->dir[1]) &&
      LookupWorkgroup (path->dir[0]))
    {
      BlueHeapFree (path->dir[0]) ;
      for (i = 1 ; i < path->num_dirs ; i++)
	path->dir[i-1] = path->dir[i] ;
      path->num_dirs-- ;
      path->dir = BlueHeapRealloc (path->dir, 
				   path->num_dirs * sizeof (BLUE_TCHAR *)) ;
    }
}

static BLUE_BOOL special (BLUE_CTCHAR ct)
{
  BLUE_BOOL ret ;

  switch (ct)
    {
    case TCHAR(' '):
    case TCHAR('<'):
    case TCHAR('>'):
    case TCHAR('#'):
    case TCHAR('%'):
    case TCHAR('{'):
    case TCHAR('}'):
    case TCHAR('|'):
    case TCHAR('\\'):
    case TCHAR('^'):
    case TCHAR('~'):
    case TCHAR('['):
    case TCHAR(']'):
    case TCHAR('`'):
    case TCHAR(';'):
    case TCHAR('/'):
    case TCHAR('?'):
    case TCHAR(':'):
    case TCHAR('@'):
    case TCHAR('='):
    case TCHAR('&'):
    case TCHAR('$'):
      ret = BLUE_TRUE ;
      break ;

    default:
      ret = BLUE_FALSE ;
      break ;
    }
  return (ret) ;
}

static BLUE_BOOL terminator (BLUE_CTCHAR c, BLUE_CTCHAR *terms)
{
  BLUE_BOOL hit ;
  BLUE_INT i ;

  if (c == '\0')
    hit = BLUE_TRUE ;
  else
    {
      for (i = 0, hit = BLUE_FALSE ; i < BlueCtstrlen(terms) && !hit ; i++)
	{
	  if (c == terms[i])
	    hit = BLUE_TRUE ;
	}
    }
  return (hit) ;
}

#define THEX(val) \
  ((val >= TCHAR('0') && val <= TCHAR('9')) ? (val - TCHAR('0')) :	\
   (val >= TCHAR('a') && val <= TCHAR('f')) ? (val - TCHAR('a') + 10) :	\
   (val >= TCHAR('A') && val <= TCHAR('F')) ? (val - TCHAR('A') + 10) : 0)

#define HEX2T(val) \
  ((val >= 0 && val <= 9) ? val + TCHAR('0') : (val - 10 + 'A'))

static BLUE_TCHAR *ParseEscaped (BLUE_CTCHAR *cursor, BLUE_CTCHAR **outcursor,
				 BLUE_CTCHAR *terms)
{
  BLUE_LPCTSTR peek ;
  BLUE_BOOL hit ;
  BLUE_LPTSTR ret ;
  BLUE_INT len ;
  BLUE_TCHAR *p ;
  BLUE_TCHAR *outstr ;

  outstr = BLUE_NULL ;
  *outcursor = cursor ;

  if (cursor == BLUE_NULL || terms == BLUE_NULL)
    ret = BLUE_NULL ;
  else
    {
      /*
       * First, find the length
       */
      for (peek = cursor, len = 0, hit = BLUE_FALSE ; !hit ; )
	{
	  BLUE_INT i ;
	  BLUE_CHAR c ;

	  if (*peek == TCHAR ('%'))
	    {
	      peek++ ;
	      for (c = 0x00, i = 0 ; *peek != TCHAR_EOS && i < 2 ; i++, peek++)
		c = (c << 4) | THEX(*peek) ;
	      len++ ;
	    }
	  else
	    {
	      if (terminator (*peek, terms))
		{
		  hit = BLUE_TRUE ;
		}
	      else
		{
		  peek++ ;
		  len++ ;
		}
	    }
	}

      if (len > 0)
	{
	  outstr = BlueHeapMalloc (sizeof (BLUE_TCHAR) * (len + 1)) ;
	  p = outstr ;

	  for (peek = cursor, hit = BLUE_FALSE ; !hit ;)
	    {
	      BLUE_INT i ;
	      BLUE_CHAR c ;

	      if (*peek == TCHAR ('%'))
		{
		  peek++ ;
		  for (c = 0x00, i = 0 ; *peek != '\0' && i < 2 ; i++, peek++)
		    c = (c << 4) | THEX(*peek) ;
		  *p++ = c ;
		}
	      else
		{
		  if (terminator (*peek, terms))
		    {
		      hit = BLUE_TRUE ;
		    }
		  else
		    {
		      *p++ = *peek++ ;
		    }
		}
	    }
	  *p = TCHAR_EOS ;
	  *outcursor = peek ;
	}
      ret = outstr ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_PATH *
BluePathInitPath (BLUE_VOID)
{
  _BLUE_PATH *path ;

  path = BlueHeapMalloc (sizeof (_BLUE_PATH)) ;
  path->type = BLUE_FS_FILE ;
  path->device = BLUE_NULL ;
  path->username = BLUE_NULL ;
  path->password = BLUE_NULL ;
  path->domain = BLUE_NULL ;
  path->port = 0 ;
  path->num_dirs = 0 ;
  path->dir = BLUE_NULL ;
  path->absolute = BLUE_FALSE ;
  path->remote = BLUE_FALSE ;

  return (path) ;
}

static BLUE_VOID BluePathUpdateType (BLUE_PATH *path)
{
#if defined(BLUE_PARAM_FS_CIFS)
  if ((BluePathType(path) == BLUE_FS_UNKNOWN ||
       BluePathType(path) == BLUE_FS_FILE) && BluePathRemote(path))
    BluePathSetType (path, BLUE_FS_CIFS) ;
#endif

  if (BluePathType(path) == BLUE_FS_UNKNOWN ||
      BluePathType(path) == BLUE_FS_FILE)
    {
#if defined(BLUE_PARAM_FS_WIN32)
      BluePathSetType (path, BLUE_FS_WIN32) ;
#elif defined(BLUE_PARAM_FS_WINCE)
      BluePathSetType (path, BLUE_FS_WINCE) ;
#elif defined(OFC_FS_DARWIN)
      BluePathSetType (path, BLUE_FS_DARWIN) ;
#elif defined(BLUE_PARAM_FS_LINUX)
      BluePathSetType (path, BLUE_FS_LINUX) ;
#elif defined(BLUE_PARAM_FS_ANDROID)
      BluePathSetType (path, BLUE_FS_ANDROID) ;
#elif defined(BLUE_PARAM_FS_FILEX)
      BluePathSetType (path, BLUE_FS_FILEX) ;
#endif
    }
}

BLUE_CORE_LIB BLUE_PATH *
BluePathCreateW (BLUE_LPCTSTR lpFileName)
{
  _BLUE_PATH *path ;
  BLUE_LPCTSTR cursor ;
  BLUE_LPCTSTR p ;
  BLUE_TCHAR *portw ;
  BLUE_CHAR *porta ;
  BLUE_LPTSTR dir ;
  BLUE_TCHAR *credentials ;
  BLUE_BOOL wild ;

  path = BluePathInitPath () ;
  cursor = lpFileName ;
  /*
   * path_parse - Parse a path into it's requisite parts.
   *
   * A path is in the form of:
   *   [uri:][\\[user[:pass[:domain]]@]server[:port]][\share][\][path\]file
   *   where uri can be smb, or drive letter
   */
  /*
   * parse a device.  The path either begins with a device or a 
   * network credential.  If it starts with a network credential, there
   * is no device.
   *
   * See if it's a network path that begins with a network terminator 
   * (\\ or //)
   */
  p = BlueCtstrtok (cursor, TSTR("\\/:")) ;
  if (*p == TCHAR_COLON)
    {
      path->device = ParseEscaped (cursor, &cursor, TSTR(":")) ;
      cursor++ ;
      if ((BlueCtstrcmp (path->device, TSTR("cifs")) == 0) ||
	  (BlueCtstrcmp (path->device, TSTR("smb")) == 0) ||
	  (BlueCtstrcmp (path->device, TSTR("proxy")) == 0))
	path->remote = BLUE_TRUE ;
      /*
       * We expect a device, colon and two separators for a uri
       * We expect a device, colon and one separator for an absolute DOS path
       * We expect a device, colong and no separators for a relative DOS path
       */
      if ((cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) &&
	  (cursor[1] == TCHAR_SLASH || cursor[1] == TCHAR_BACKSLASH))
	{
	  cursor += 2 ;
	  path->absolute = BLUE_TRUE ;
	}
      else if (cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH)
	{
	  cursor ++ ;
	  path->absolute = BLUE_TRUE ;
	}
    }
  else if ((cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) &&
	   (cursor[1] == TCHAR_SLASH || cursor[1] == TCHAR_BACKSLASH))
    {
      /* UNC path */
      cursor += 2 ;
      path->remote = BLUE_TRUE ;
      path->absolute = BLUE_TRUE ;
    }
  else if (cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) 
    {
      cursor++ ;
      path->absolute = BLUE_TRUE ;
    }

  path->num_dirs = 0 ;
  if (path->remote)
    {
      /*
       * There's an authority (remote access)
       * Find out if there's a credential.  There will be an unescapedd "@"
       */
      credentials = ParseEscaped (cursor, &p, TSTR("@")) ;
      BlueHeapFree (credentials) ;

      if (*p == TCHAR_AMP)
	{
	  /*
	   * we have user:pass:domain
	   */
	  path->username = ParseEscaped (cursor, &cursor, TSTR(":@")) ;
	  if (*cursor == TCHAR_COLON)
	    {
	      cursor++ ;
	      /*
	       * we have a password:domain
	       */
	      path->password = ParseEscaped (cursor, &cursor, TSTR(":@")) ;
	    }

	  if (*cursor == TCHAR_COLON)
	    {
	      cursor++ ;
	      /*
	       * we have a domain
	       */
	      path->domain = ParseEscaped (cursor, &cursor, TSTR("@")) ;
	    }
	  cursor++ ;
	}
    }
      
  while (*cursor != TCHAR_EOS)
    {
      dir = ParseEscaped (cursor, &cursor, TSTR("\\/:")) ;

      if (dir != BLUE_NULL)
	{
	  wild = BluePathIsWild (dir) ;

	  if (BlueCtstrcmp (dir, TSTR("..")) == 0)
	    {
	      if (path->num_dirs > 0)
		{
		  path->num_dirs-- ;
		  BlueHeapFree (path->dir[path->num_dirs]) ;
		  path->dir = BlueHeapRealloc (path->dir,
					       sizeof (BLUE_LPCTSTR *) *
					       (path->num_dirs)) ;
		}
	      BlueHeapFree (dir) ;
	    }
	  else if (BlueCtstrcmp (dir, TSTR(".")) == 0)
	    {
	      /* Skip it */
	      BlueHeapFree (dir) ;
	    }
	  /*
	   * if we're remote and if dir index is 1 we're looking at a share or
	   * potentially a server if dir index 0 is a workgroup.  In the latter
	   * case, if the current item is not a wildcard, we want to strip the 
	   * workgroup. 
	   */
	  else if (path->remote && path->num_dirs == 1 && !wild &&
		   LookupWorkgroup (path->dir[0]))
	    {
	      /* 
	       * Strip the workgroup. 
	       * Free the old one, and replace it with this one.
	       */
	      BlueHeapFree (path->dir[0]) ;
	      path->dir[0] = dir ;
	    }
	  else
	    {
	      path->dir = BlueHeapRealloc (path->dir,
					   sizeof (BLUE_LPCTSTR *) *
					   (path->num_dirs + 1)) ;
	      path->dir[path->num_dirs] = dir ;
	      path->num_dirs++ ;
	    }

	  if ((*cursor == TCHAR(':')) && path->remote && (path->num_dirs == 1))
	    {
	      cursor++ ;
	      portw = ParseEscaped (cursor, &cursor, TSTR("\\/")) ;
	      porta = BlueCtstr2cstr (portw) ;
	      path->port = (BLUE_INT) BlueCstrtoul (porta, BLUE_NULL, 10) ;
	      BlueHeapFree (portw) ;
	      BlueHeapFree (porta) ;
	    }
	}
      if (*cursor != TCHAR_EOS)
	cursor++ ;
    }
       
  BluePathUpdateType ((BLUE_PATH *) path) ;

  return ((BLUE_PATH *) path) ;
}

BLUE_CORE_LIB BLUE_PATH *
BluePathCreateA (BLUE_LPCSTR lpFileName)
{
  _BLUE_PATH *ret ;
  BLUE_TCHAR *lptFileName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BluePathCreateW (lptFileName) ;
  BlueHeapFree (lptFileName) ;
  return ((BLUE_PATH *) ret) ;
}

BLUE_CORE_LIB BLUE_VOID 
BluePathDelete (BLUE_PATH *_path)
{
  BLUE_UINT i ;
  _BLUE_PATH *path = (_BLUE_PATH *) _path;

  BlueHeapFree (path->device) ;
  BlueHeapFree (path->username) ;
  BlueHeapFree (path->password) ;
  BlueHeapFree (path->domain) ;
  for (i = 0 ; i < path->num_dirs ; i++)
    {
      BlueHeapFree (path->dir[i]) ;
    }
  BlueHeapFree (path->dir) ;
  BlueHeapFree (path) ;
}

static BLUE_SIZET 
BluePathOutChar (BLUE_TCHAR c, BLUE_LPTSTR *dest, BLUE_SIZET *rem)
{
  if (*rem > 0)
    {
      (*rem)-- ;
      *(*dest)++ = c ;
    }
  return (1) ;
}


static BLUE_SIZET 
BluePathOutEscaped (BLUE_LPCTSTR str, BLUE_LPTSTR *filename, BLUE_SIZET *rem)
{
  BLUE_LPCTSTR p ;
  BLUE_SIZET len ;

  len = 0 ;
  for (p = str ; *p != TCHAR_EOS ; p++)
    {
      if (special(*p))
	{
	  BLUE_TCHAR c ;
	  
	  len += BluePathOutChar (TCHAR('%'), filename, rem) ;
	  c = *p >> 4 ;
	  len += BluePathOutChar (HEX2T(c), filename, rem) ;
	  c = *p & 0x0F ;
	  len += BluePathOutChar (HEX2T(c), filename, rem) ;
	}
      else
	len += BluePathOutChar (*p, filename, rem) ;
    }
    
  return (len) ;
}

static BLUE_SIZET 
BluePathOutEscapedLC (BLUE_LPCTSTR str, BLUE_LPTSTR *filename, BLUE_SIZET *rem)
{
  BLUE_LPCTSTR p ;
  BLUE_SIZET len ;

  len = 0 ;
  for (p = str ; *p != TCHAR_EOS ; p++)
    {
      if (special(*p))
	{
	  BLUE_TCHAR c ;
	  
	  len += BluePathOutChar (TCHAR('%'), filename, rem) ;
	  c = *p >> 4 ;
	  len += BluePathOutChar (HEX2T(c), filename, rem) ;
	  c = *p & 0x0F ;
	  len += BluePathOutChar (HEX2T(c), filename, rem) ;
	}
      else
	len += BluePathOutChar (BLUE_C_TOLOWER(*p), filename, rem) ;
    }
    
  return (len) ;
}

static BLUE_SIZET 
BluePathOutStr (BLUE_LPCTSTR str, BLUE_LPTSTR *filename, BLUE_SIZET *rem)
{
  BLUE_LPCTSTR p ;
  BLUE_SIZET len ;

  len = 0 ;
  for (p = str ; *p != TCHAR_EOS ; p++)
    {
      len += BluePathOutChar (*p, filename, rem) ;
    }
    
  return (len) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BluePathPrintW (BLUE_PATH *_path, BLUE_LPTSTR *filename, BLUE_SIZET *rem)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_TCHAR delimeter ;
  BLUE_SIZET len ;
  BLUE_UINT i ;

  switch (path->type)
    {
    case BLUE_FS_WIN32:
    case BLUE_FS_FILEX:
      delimeter = TCHAR_BACKSLASH ;
      break ;

    case BLUE_FS_CIFS:
#if defined(__ANDROID__)
      delimeter = TCHAR_SLASH ;
#else
      delimeter = TCHAR_BACKSLASH ;
#endif

    case BLUE_FS_DARWIN:
    case BLUE_FS_LINUX:
    case BLUE_FS_ANDROID:
      delimeter = TCHAR_SLASH ;
      break ;

    case BLUE_FS_UNKNOWN:
    case BLUE_FS_FILE:
    default:
#if defined(BLUE_PARAM_FS_WIN32)
      delimeter = TCHAR_BACKSLASH ;
#else
      delimeter = TCHAR_SLASH ;
#endif
      break ;
    }
  /*
   * Need to do an update remainder kind of thing
   */
  len = 0 ;

  if (path->device != BLUE_NULL)
    {
      len += BluePathOutEscapedLC (path->device, filename, rem) ;
      len += BluePathOutChar (TCHAR_COLON, filename, rem) ;
      if (path->absolute)
	{
	  len += BluePathOutChar (delimeter, filename, rem) ;
	  len += BluePathOutChar (delimeter, filename, rem) ;
	}
    }
  else if (path->remote)
    {
      len += BluePathOutChar (delimeter, filename, rem) ;
      len += BluePathOutChar (delimeter, filename, rem) ;
    }
  else if (path->absolute)
    {
      if (path->num_dirs > 0)
	len += BluePathOutChar (delimeter, filename, rem) ;
    }      

  if (path->remote)
    {
      if (path->username != BLUE_NULL)
	{
	  len += BluePathOutEscaped (path->username, filename, rem) ;

	  if (path->password != BLUE_NULL || path->domain != BLUE_NULL)
	    {
	      len += BluePathOutChar (TCHAR_COLON, filename, rem) ;

	      if (path->password != BLUE_NULL && 
		  path->password[0] != TCHAR_EOS)
		{
		  len += BluePathOutEscaped (path->password, filename, rem) ;
		  
		  if (path->domain != BLUE_NULL)
		    {
		      len += BluePathOutChar (TCHAR_COLON, filename, rem) ;
		    }
		}

	      if (path->domain != BLUE_NULL)
		{
		  len += BluePathOutEscaped (path->domain, filename, rem) ;
		}
	    }
	  len += BluePathOutChar (TCHAR_AMP, filename, rem) ;
	}
    }

  for (i = 0 ; i < path->num_dirs ; i++)
    {
      if (path->dir[i] != BLUE_NULL && 
	  BlueCtstrcmp(path->dir[i], TSTR("")) != 0)
	{
	  len += BluePathOutStr (path->dir[i], filename, rem) ;

	  if (path->remote && (i == 0) && (path->port != 0))
	    {
	      BLUE_SIZET count ;
	      BLUE_CHAR *porta ;
	      BLUE_TCHAR *portw ;

	      count = 0 ;
	      count = BlueCsnprintf (BLUE_NULL, count, "%d", path->port) ;
	      porta = BlueHeapMalloc ((count+1)*sizeof(BLUE_CHAR)) ;
	      BlueCsnprintf (porta, count+1, "%d", path->port) ;
	      portw = BlueCcstr2tstr (porta) ;
	      len += BluePathOutChar (TCHAR(':'), filename, rem) ;
	      len += BluePathOutStr (portw, filename, rem) ;
	      BlueHeapFree (porta) ;
	      BlueHeapFree (portw) ;
	    }

	  if ((i + 1) < path->num_dirs)
	    len += BluePathOutChar (delimeter, filename, rem) ;
	}
    }

  BluePathOutChar (TCHAR_EOS, filename, rem) ;
  return (len) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BluePathPrintA (BLUE_PATH *_path, BLUE_LPSTR *filename, BLUE_SIZET *rem)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_SIZET ret ;
  BLUE_SIZET orig_rem ;
  BLUE_TCHAR *tfilename ;
  BLUE_TCHAR *ptfilename ;
  BLUE_CHAR *pfilename ;
  BLUE_INT i ;

  orig_rem = *rem ;
  tfilename = BlueHeapMalloc (*rem * sizeof (BLUE_TCHAR)) ;
  ptfilename = tfilename ;
  ret = BluePathPrintW (path, &ptfilename, rem) ;
  if (filename != BLUE_NULL)
    {
      pfilename = *filename ;
      for (i = 0 ; i < BLUE_C_MIN (orig_rem, ret + 1) ; i++)
	*pfilename++ = (BLUE_CHAR) tfilename[i] ;
    }
  BlueHeapFree (tfilename) ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_FS_TYPE BluePathType (BLUE_PATH *_path) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  return (path->type) ;
}

/*
 * Inner workings of BluePathMapW.  Returns the resulting path
 * A map is a device (gen:/temp) or a server name (//gen/temp)
 */
BLUE_CORE_LIB BLUE_PATH *BlueMapPath (BLUE_LPCTSTR lpFileName,
				      BLUE_LPTSTR *lppMappedName) 
{
  BLUE_PATH *path ;
  BLUE_PATH *map ;
  BLUE_SIZET len ;
  BLUE_LPTSTR cursor ;
#if defined(AUTHENTICATE)
  BLUE_LPCTSTR server ;
  BLUE_BOOL updated ;
  BLUE_LPCTSTR previous_server ;
#endif
  
  path = BluePathCreateW (lpFileName) ;

#if defined(AUTHENTICATE)
  previous_server = BLUE_NULL ;
  do
    {
      updated = BLUE_FALSE ;
      server = BluePathDevice(path) ;

      if (server == BLUE_NULL)
	server = BluePathServer(path) ;

      if (server != BLUE_NULL)
	{
	  /*
	   * We don't want to get stuck in a loop
	   * protect against:
	   * //aserver -> //user:domain:password@aserver
	   *
	   * although something like:
	   * //aserver -> //bserver
	   * //bserver -> //aserver
	   * can still happen
	   */
	  if (previous_server == BLUE_NULL || 
	      BlueCtstrcmp (server, previous_server) != 0)
	    {
	      previous_server = server ;
	      map = BluePathMapDeviceW (server) ;

	      if (map != BLUE_NULL)
		{
		  BluePathUpdate (path, map) ;
		  updated = BLUE_TRUE ;
		}
	    }
	}
    }
  while (updated) ;
#else
  for (map = BluePathMapDeviceW (BluePathDevice(path)) ;
       map != BLUE_NULL ;
       map = BluePathMapDeviceW (BluePathDevice(map)))
    {
      BluePathUpdate (path, map) ;
    } ;
#endif

  BluePathUpdateType (path) ;

  if (lppMappedName != BLUE_NULL)
    {
      len = 0 ;
      len = BluePathPrintW (path, BLUE_NULL, &len) + 1 ;

      *lppMappedName = BlueHeapMalloc (len * sizeof(BLUE_TCHAR)) ;

      cursor = *lppMappedName ;
      BluePathPrintW (path, &cursor, &len) ;
    }

  return (path) ;
}
  
/*
 * A map is a device (i.e. gen:/temp) or a server name: (i.e. //gen/temp
 */
BLUE_CORE_LIB BLUE_VOID 
BluePathMapW (BLUE_LPCTSTR lpFileName, BLUE_LPTSTR *lppMappedName,
	      BLUE_FS_TYPE *filesystem)
{
  BLUE_PATH *path ;
  
  path = BlueMapPath (lpFileName, lppMappedName) ;

  if (filesystem != BLUE_NULL)
    *filesystem = BluePathType(path) ;
  BluePathDelete (path) ;
}

BLUE_CORE_LIB BLUE_VOID 
BluePathMapA (BLUE_LPCSTR lpFileName, BLUE_LPSTR *lppMappedName,
	      BLUE_FS_TYPE *filesystem)
{
  BLUE_TCHAR *lptFileName ;
  BLUE_TCHAR *lptMappedName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  lptMappedName = BLUE_NULL ;
  BluePathMapW (lptFileName, &lptMappedName, filesystem) ;
  if (lppMappedName != BLUE_NULL)
    *lppMappedName = BlueCtstr2cstr (lptMappedName) ;
  BlueHeapFree (lptFileName) ;
  BlueHeapFree (lptMappedName) ;
}

BLUE_CORE_LIB BLUE_VOID 
BluePathInit (BLUE_VOID)
{
  BLUE_INT i ;

  lockPath = BlueLockInit () ;

  for (i = 0 ; i < BLUE_PARAM_MAX_MAPS ; i++)
    {
      BluePathMaps[i].lpDevice = BLUE_NULL ;
      BluePathMaps[i].lpDesc = BLUE_NULL ;
      BluePathMaps[i].map = BLUE_NULL ;
      BluePathMaps[i].thumbnail = BLUE_FALSE ;
    }
}      

BLUE_CORE_LIB BLUE_VOID 
BluePathDestroy (BLUE_VOID)
{
  BLUE_INT i ;

  for (i = 0 ; i < BLUE_PARAM_MAX_MAPS ; i++)
    {
      if (BluePathMaps[i].lpDevice != BLUE_NULL)
	BluePathDeleteMapW (BluePathMaps[i].lpDevice);
    }

  BlueLockDestroy (lockPath) ;
}      

/**
 * Add a map for a virtual path
 *
 * \param lpVirtual
 * Virtual Patho
 *
 * \param map
 * path to use for translating virtual path
 */
BLUE_CORE_LIB BLUE_BOOL 
BluePathAddMapW (BLUE_LPCTSTR lpDevice, BLUE_LPCTSTR lpDesc, 
		 BLUE_PATH *_map, BLUE_FS_TYPE fsType, BLUE_BOOL thumbnail)
{
  _BLUE_PATH *map = (_BLUE_PATH *) _map ;
  BLUE_INT i ;
  BLUE_BOOL ret ;
  PATH_MAP_ENTRY *free ;
  BLUE_TCHAR *lc ;
  /*
   * Search for a free entry and make sure the device is not already mapped
   */
  ret = BLUE_TRUE ;
  free = BLUE_NULL ;

  BlueLock (lockPath) ;

  for (i = 0 ; i < BLUE_PARAM_MAX_MAPS && ret == BLUE_TRUE ; i++)
    {
      if (BluePathMaps[i].lpDevice == BLUE_NULL)
        {
          if (free == BLUE_NULL)
            free = &BluePathMaps[i] ;
        }
      else
        {
          if (BlueCtstrcasecmp (BluePathMaps[i].lpDevice, lpDevice) == 0)
            {
              ret = BLUE_FALSE ;
            }
        }
    }

  if (ret == BLUE_TRUE)
    {
      if (free == BLUE_NULL)
	{
	  ret = BLUE_FALSE ;
	}
      else
	{
	  if (lpDevice == BLUE_NULL)
	    ret = BLUE_FALSE ;
	  else
	    {
	      map->type = fsType ;
	      free->lpDevice = BlueCtstrdup (lpDevice) ;
	      for (lc = free->lpDevice ; *lc != TCHAR_EOS ; lc++)
		*lc = BLUE_C_TOLOWER(*lc) ;
	      free->lpDesc = BlueCtstrdup (lpDesc) ;
	      free->map = map ;
	      free->thumbnail = thumbnail ;
	    }
	}
    }
  BlueUnlock (lockPath) ;
  return (ret) ;
}
  
BLUE_CORE_LIB BLUE_BOOL 
BluePathAddMapA (BLUE_LPCSTR lpDevice, BLUE_LPCSTR lpDesc,
		 BLUE_PATH *_map, BLUE_FS_TYPE fsType, BLUE_BOOL thumbnail)
{
  BLUE_BOOL ret ;
  BLUE_TCHAR *lptDevice ;
  BLUE_TCHAR *lptDesc ;

  lptDevice = BlueCcstr2tstr (lpDevice) ;
  lptDesc = BlueCcstr2tstr (lpDesc) ;
  ret = BluePathAddMapW (lptDevice, lptDesc, _map, fsType, thumbnail) ;
  BlueHeapFree (lptDevice) ;
  BlueHeapFree (lptDesc) ;
  return (ret) ;
}

/**
 * Delete a map
 *
 * \param lpVirtual
 * Virtual path of map to delete
 */
BLUE_CORE_LIB BLUE_VOID 
BluePathDeleteMapW (BLUE_LPCTSTR lpDevice) 
{
  BLUE_INT i ;
  PATH_MAP_ENTRY *pathEntry ;

  /*
   * Search for a free entry and make sure the device is not already mapped
   */
  pathEntry = BLUE_NULL ;

  BlueLock (lockPath) ;

  for (i = 0 ; i < BLUE_PARAM_MAX_MAPS && pathEntry == BLUE_NULL ; i++)
    {
      if (BluePathMaps[i].lpDevice != BLUE_NULL &&
          BlueCtstrcasecmp (BluePathMaps[i].lpDevice, lpDevice) == 0)
        pathEntry = &BluePathMaps[i] ;
    }

  if (pathEntry != BLUE_NULL)
    {
      BlueHeapFree (pathEntry->lpDevice) ;
      BlueHeapFree (pathEntry->lpDesc) ;
      BluePathDelete (pathEntry->map) ;
      pathEntry->lpDevice = BLUE_NULL ;
      pathEntry->map = BLUE_NULL ;
    }

  BlueUnlock (lockPath) ;
}

BLUE_CORE_LIB BLUE_VOID 
BluePathDeleteMapA (BLUE_LPCSTR lpDevice) 
{
  BLUE_TCHAR *lptDevice ;

  lptDevice = BlueCcstr2tstr (lpDevice) ;
  BluePathDeleteMapW (lptDevice) ;
  BlueHeapFree (lptDevice) ;
}

BLUE_CORE_LIB BLUE_VOID
BluePathGetMapW (BLUE_INT idx, BLUE_LPCTSTR *lpDevice, 
		 BLUE_LPCTSTR *lpDesc, BLUE_PATH **map,
		 BLUE_BOOL *thumbnail)
{
  BlueLock (lockPath) ;

  *lpDevice = BluePathMaps[idx].lpDevice ;
  *lpDesc = BluePathMaps[idx].lpDesc ;
  *map = BluePathMaps[idx].map ;
  *thumbnail = BluePathMaps[idx].thumbnail ;
  BlueUnlock (lockPath) ;
}
  
BLUE_CORE_LIB BLUE_PATH *
BluePathMapDeviceW (BLUE_LPCTSTR lpDevice)
{
  BLUE_INT i ;
  PATH_MAP_ENTRY *pathEntry ;
  BLUE_PATH *map ;
  BLUE_CTCHAR *p ;
  BLUE_TCHAR *lc ;
  BLUE_SIZET len ;
  BLUE_LPTSTR tstrDevice ;

  /*
   * Search for a free entry and make sure the device is not already mapped
   */
  map = BLUE_NULL ;
  if (lpDevice != BLUE_NULL)
    {
      /*
       * Peel off just the device portion.  Normal users pass in the
       * device only anyway but this allows the routine to be more general.
       * We prefer sending in a full path when determining if the path
       * is served by one of our maps
       */
      p = BlueCtstrtok (lpDevice, TSTR(":")) ;
      len = ((BLUE_ULONG_PTR) p - (BLUE_ULONG_PTR) lpDevice) /
	sizeof (BLUE_TCHAR) ;
      tstrDevice = BlueHeapMalloc ((len + 1) * sizeof (BLUE_TCHAR)) ;

      BlueCtstrncpy (tstrDevice, lpDevice, len) ;
      tstrDevice[len] = TCHAR_EOS ;
	
      for (lc = tstrDevice ; *lc != TCHAR_EOS ; lc++)
	*lc = BLUE_C_TOLOWER(*lc) ;

      pathEntry = BLUE_NULL ;

      for (i = 0 ; i < BLUE_PARAM_MAX_MAPS && pathEntry == BLUE_NULL ; i++)
	{
	  if (BluePathMaps[i].lpDevice != BLUE_NULL &&
	      BlueCtstrcasecmp (BluePathMaps[i].lpDevice, tstrDevice) == 0)
	    {
	      pathEntry = &BluePathMaps[i] ;
	    }
	}

      BlueHeapFree (tstrDevice) ;
      if (pathEntry != BLUE_NULL)
	map = pathEntry->map ;	
    }
  return (map) ;
}

BLUE_CORE_LIB BLUE_PATH *
BluePathMapDeviceA (BLUE_LPCSTR lpDevice)
{
  _BLUE_PATH *ret ;
  BLUE_TCHAR *lptDevice ;

  lptDevice = BlueCcstr2tstr (lpDevice) ;
  ret = BluePathMapDeviceW (lptDevice) ;
  BlueHeapFree (lptDevice) ;
  return ((BLUE_PATH *) ret) ;
}

BLUE_CORE_LIB BLUE_SIZET BluePathMakeURLW (BLUE_LPTSTR *filename,
					   BLUE_SIZET *rem,
					   BLUE_LPCTSTR username,
					   BLUE_LPCTSTR password,
					   BLUE_LPCTSTR domain,
					   BLUE_LPCTSTR server,
					   BLUE_LPCTSTR share,
					   BLUE_LPCTSTR path,
					   BLUE_LPCTSTR file)
  
{
  BLUE_TCHAR delimeter ;
  BLUE_SIZET len ;

  delimeter = TCHAR_BACKSLASH ;
  /*
   * Need to do an update remainder kind of thing
   */
  len = 0 ;

  len += BluePathOutChar (delimeter, filename, rem) ;
  len += BluePathOutChar (delimeter, filename, rem) ;

  if (username != BLUE_NULL)
    {
      len += BluePathOutEscaped (username, filename, rem) ;

      if (password != BLUE_NULL || domain != BLUE_NULL)
	{
	  len += BluePathOutChar (TCHAR_COLON, filename, rem) ;

	  if (password != BLUE_NULL && password[0] != TCHAR_EOS)
	    {
	      len += BluePathOutEscaped (password, filename, rem) ;
		  
	      if (domain != BLUE_NULL)
		{
		  len += BluePathOutChar (TCHAR_COLON, filename, rem) ;
		}
	    }

	  if (domain != BLUE_NULL)
	    {
	      len += BluePathOutEscaped (domain, filename, rem) ;
	    }
	}
      len += BluePathOutChar (TCHAR_AMP, filename, rem) ;
    }

  if (server != BLUE_NULL)
    {
      len += BluePathOutStr (server, filename, rem) ;
      len += BluePathOutChar (delimeter, filename, rem) ;
    }

  if (share != BLUE_NULL)
    {
      len += BluePathOutStr (share, filename, rem) ;
      len += BluePathOutChar (delimeter, filename, rem) ;
    }

  if (path != BLUE_NULL)
    {
      len += BluePathOutStr (path, filename, rem) ;
      len += BluePathOutChar (delimeter, filename, rem) ;
    }

  if (file != BLUE_NULL)
    {
      len += BluePathOutStr (file, filename, rem) ;
    }

  BluePathOutChar (TCHAR_EOS, filename, rem) ;
  return (len) ;
}

BLUE_CORE_LIB BLUE_SIZET BluePathMakeURLA (BLUE_LPSTR *filename,
					   BLUE_SIZET *rem,
					   BLUE_LPCSTR username,
					   BLUE_LPCSTR password,
					   BLUE_LPCSTR domain,
					   BLUE_LPCSTR server,
					   BLUE_LPCSTR share,
					   BLUE_LPCSTR path,
					   BLUE_LPCSTR file)
{
  BLUE_LPTSTR tfilename ;
  BLUE_LPTSTR tcursor ;
  BLUE_LPTSTR tusername ;
  BLUE_LPTSTR tpassword ;
  BLUE_LPTSTR tdomain ;
  BLUE_LPTSTR tserver ;
  BLUE_LPTSTR tshare ;
  BLUE_LPTSTR tpath ;
  BLUE_LPTSTR tfile ;
  BLUE_SIZET len ;
  BLUE_SIZET orig_rem ;
  BLUE_LPSTR pfilename ;
  BLUE_INT i ;

  tusername = BlueCcstr2tstr (username) ;
  tpassword = BlueCcstr2tstr (password) ;
  tdomain = BlueCcstr2tstr (domain) ;
  tserver = BlueCcstr2tstr (server) ;
  tshare = BlueCcstr2tstr (share) ;
  tpath = BlueCcstr2tstr (path) ;
  tfile = BlueCcstr2tstr (file) ;

  orig_rem = *rem ;
  tfilename = BlueHeapMalloc ((*rem+1) * sizeof (BLUE_TCHAR)) ;
  tcursor = tfilename ;

  len = BluePathMakeURLW (&tcursor, rem, tusername, tpassword, tdomain,
			  tserver, tshare, tpath, tfile) ;

  pfilename = *filename ;
  for (i = 0 ; i < BLUE_C_MIN (orig_rem, len + 1) ; i++)
    *pfilename++ = (BLUE_CHAR) tfilename[i] ;

  BlueHeapFree (tusername) ;
  BlueHeapFree (tpassword) ;
  BlueHeapFree (tdomain) ;
  BlueHeapFree (tserver) ;
  BlueHeapFree (tshare) ;
  BlueHeapFree (tpath) ;
  BlueHeapFree (tfile) ;
  BlueHeapFree (tfilename) ;

  return (len) ;
}

#if defined(AUTHENTICATE)
BLUE_CORE_LIB BLUE_VOID 
BluePathUpdateCredentialsW (BLUE_LPCTSTR filename, BLUE_LPCTSTR username,
			    BLUE_LPCTSTR password, BLUE_LPCTSTR domain) 
{
  BLUE_PATH *path ;
  BLUE_PATH *map ;
  _BLUE_PATH *_map ;
  BLUE_LPCTSTR server ;

  path = BluePathCreateW (filename) ;

  server = BluePathServer(path) ;

  if (server != BLUE_NULL)
    {
      map = BluePathMapDeviceW (BluePathServer(path)) ;

      if (map == BLUE_NULL)
	{
	  /*
	   * We just want the path to contain a server (and authentication)
	   */
	  BluePathFreeDirs(path) ;
	  /*
	   * Add server to data base
	   */
	  BluePathAddMapW (BluePathServer(path), TSTR("Server"), 
			   path, BLUE_FS_CIFS, BLUE_TRUE) ;
	  map = path ;
	}

      BlueLock (lockPath) ;

      _map = (_BLUE_PATH *) map ;

      if (_map->username != BLUE_NULL)
	BlueHeapFree (_map->username) ;
      _map->username = BlueCtstrdup (username) ;
      if (_map->password != BLUE_NULL)
	{
	  BlueCmemset (_map->password, '\0', 
		       BlueCtstrlen (_map->password) * sizeof (BLUE_TCHAR)) ;
	  BlueHeapFree (_map->password) ;
	}
      _map->password = BlueCtstrdup (password) ;
      if (_map->domain != BLUE_NULL)
	BlueHeapFree (_map->domain) ;
      _map->domain = BlueCtstrdup (domain) ;

      BlueUnlock (lockPath) ;
    }
}
#else  
BLUE_CORE_LIB BLUE_VOID 
BluePathUpdateCredentialsW (BLUE_PATH *_path, BLUE_LPCTSTR username,
 			    BLUE_LPCTSTR password, BLUE_LPCTSTR domain) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

   BlueLock (lockPath) ;
 
  if (path->username != BLUE_NULL)
    BlueHeapFree (path->username) ;
  path->username = BlueCtstrdup (username) ;
  if (path->password != BLUE_NULL)
     {
      BlueCmemset (path->password, '\0', 
		   BlueCtstrlen (path->password) * sizeof (BLUE_TCHAR)) ;
      BlueHeapFree (path->password) ;
     }
  path->password = BlueCtstrdup (password) ;
  if (path->domain != BLUE_NULL)
    BlueHeapFree (path->domain) ;
  path->domain = BlueCtstrdup (domain) ;

  BlueUnlock (lockPath) ;
}
#endif


#if defined(AUTHENTICATE)
BLUE_CORE_LIB BLUE_VOID 
BluePathUpdateCredentialsA (BLUE_LPCSTR filename, BLUE_LPCSTR username,
			    BLUE_LPCSTR password, BLUE_LPCSTR domain) 
{
  BLUE_TCHAR *tusername ;
  BLUE_TCHAR *tpassword ;
  BLUE_TCHAR *tdomain ;
  BLUE_TCHAR *tfilename ;

  tfilename = BlueCcstr2tstr (filename) ;
  tusername = BlueCcstr2tstr (username) ;
  tpassword = BlueCcstr2tstr (password) ;
  tdomain = BlueCcstr2tstr (domain) ;
  BluePathUpdateCredentialsW (tfilename, tusername, tpassword, tdomain) ;
  BlueHeapFree (tfilename) ;
  BlueHeapFree (tusername) ;
  BlueHeapFree (tpassword) ;
  BlueHeapFree (tdomain) ;
}
#else
BLUE_CORE_LIB BLUE_VOID 
BluePathUpdateCredentialsA (BLUE_PATH *path, BLUE_LPCSTR username,
			    BLUE_LPCSTR password, BLUE_LPCSTR domain) 
{
  BLUE_TCHAR *tusername ;
  BLUE_TCHAR *tpassword ;
  BLUE_TCHAR *tdomain ;
 
  tusername = BlueCcstr2tstr (username) ;
  tpassword = BlueCcstr2tstr (password) ;
  tdomain = BlueCcstr2tstr (domain) ;
  BluePathUpdateCredentialsW (path, tusername, tpassword, tdomain) ;
  BlueHeapFree (tusername) ;
  BlueHeapFree (tpassword) ;
  BlueHeapFree (tdomain) ;
}
#endif

BLUE_CORE_LIB BLUE_VOID
BluePathGetRootW (BLUE_CTCHAR *lpFileName, BLUE_TCHAR **lpRootName,
		  BLUE_FS_TYPE *filesystem)
{
  BLUE_TCHAR *lpMappedPath ;
  BLUE_TCHAR *lpMappedName ;
  _BLUE_PATH *rootpath ;
  _BLUE_PATH *path ;
  BLUE_SIZET len ;

  BluePathMapW (lpFileName, &lpMappedName, filesystem) ;
  path = BluePathCreateW (lpMappedName) ;

  if (path->remote)
    {
      rootpath = BluePathCreateW (TSTR("\\")) ;
      rootpath->absolute = BLUE_TRUE ;
      rootpath->device = BlueCtstrdup (path->device) ;
      /*
       * Then the share is part of the root
       */
      rootpath->remote = BLUE_TRUE ;
      rootpath->port = path->port ;
      rootpath->username = BlueCtstrdup (path->username) ;
      rootpath->password = BlueCtstrdup (path->password) ;
      rootpath->domain = BlueCtstrdup (path->domain) ;
      if (path->num_dirs > 1)
	{
	  rootpath->num_dirs = 2 ;
	  rootpath->dir = BlueHeapMalloc (sizeof (BLUE_LPCTSTR *) * 
					  rootpath->num_dirs) ;
	  rootpath->dir[0] = BlueCtstrdup (path->dir[0]) ;
	  rootpath->dir[1] = BlueCtstrdup (path->dir[1]) ;
	}
    }
  else
    rootpath = BluePathCreateW (lpMappedName) ;

  len = 0 ;
  len = BluePathPrintW (rootpath, BLUE_NULL, &len) ;

  if (rootpath->type == BLUE_FS_WIN32)
    /* Terminating slash */
    len++ ;
  /* EOS */
  len++ ;
  *lpRootName = BlueHeapMalloc (len * sizeof (BLUE_TCHAR)) ;
  lpMappedPath = *lpRootName ;
  len = BluePathPrintW (rootpath, &lpMappedPath, &len) ;
  if (rootpath->type == BLUE_FS_WIN32)
    (*lpRootName)[len++] = TCHAR_BACKSLASH ;
  (*lpRootName)[len] = TCHAR_EOS ;
  BluePathDelete (rootpath) ;
  BluePathDelete (path) ;
  BlueHeapFree (lpMappedName) ;
}

BLUE_CORE_LIB BLUE_VOID
BluePathGetRootA (BLUE_CCHAR *lpFileName, BLUE_CHAR **lpRoot,
		  BLUE_FS_TYPE *filesystem)
{
  BLUE_TCHAR *lptFileName ;
  BLUE_TCHAR *lptRoot ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  BluePathGetRootW (lptFileName, &lptRoot, filesystem) ;
  *lpRoot = BlueCtstr2cstr (lptRoot) ;
  BlueHeapFree (lptFileName) ;
  BlueHeapFree (lptRoot) ;
}

BLUE_CORE_LIB BLUE_BOOL BluePathRemote (BLUE_PATH *_path) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (path->remote)
    ret = BLUE_TRUE ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_INT BluePathPort (BLUE_PATH *_path) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_INT port ;

  port = path->port ;

  return (port) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetPort (BLUE_PATH *_path, BLUE_INT port) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  path->port = port ;
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathServer (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_LPCTSTR server ;

  server = BLUE_NULL ;

  if (path->remote && path->num_dirs > 0)
    server = path->dir[0] ;

  return (server) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathFreeServer (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote && path->num_dirs > 0)
    {
      BlueHeapFree (path->dir[0]) ;
      path->dir[0] = BLUE_NULL ;
    }
}

BLUE_CORE_LIB BLUE_VOID
BluePathSetServer (BLUE_PATH *_path, BLUE_LPTSTR server)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->num_dirs > 0)
	{
	  BlueHeapFree (path->dir[0]) ;
	}
      else
	{
	  path->num_dirs++ ;
	  path->dir = BlueHeapRealloc (path->dir, (path->num_dirs *
						   sizeof (BLUE_LPCSTR))) ;
	}
      path->dir[0] = (BLUE_LPTSTR) server ;
    }
  else
    BlueHeapFree (server) ;
}

BLUE_CORE_LIB BLUE_VOID
BluePathSetShare (BLUE_PATH *_path, BLUE_LPCTSTR share)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->num_dirs > 1)
	{
	  BlueHeapFree (path->dir[1]) ;
	}
      else
	{
	  path->num_dirs++ ;
	  path->dir = BlueHeapRealloc (path->dir, (path->num_dirs *
						   sizeof (BLUE_LPCSTR))) ;
	}
      path->dir[1] = BlueCtstrdup (share) ;
    }
}

BLUE_CORE_LIB BLUE_VOID
BluePathSetFilename (BLUE_PATH *_path, BLUE_LPCTSTR filename)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_UINT prefix ;

  if (path->remote)
    prefix = 2 ;
  else
    prefix = 0 ;

  if (path->num_dirs > prefix)
    {
      BlueHeapFree (path->dir[path->num_dirs - 1]) ;
    }
  else
    {
      path->num_dirs++ ;
      path->dir = BlueHeapRealloc (path->dir, (path->num_dirs *
					       sizeof (BLUE_LPCSTR))) ;
    }
  path->dir[path->num_dirs - 1] = BlueCtstrdup (filename) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BluePathServerCmp (BLUE_PATH *_path, BLUE_LPCTSTR server) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (path->remote && path->num_dirs > 0 && path->dir[0] != BLUE_NULL)
    {
      if (BlueCtstrcmp (path->dir[0], server) == 0)
	ret = BLUE_TRUE ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_BOOL 
BluePathPortCmp (BLUE_PATH *_path, BLUE_UINT16 port) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (path->remote && (path->port == port))
    ret = BLUE_TRUE ;
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathShare (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_LPCTSTR share ;

  share = BLUE_NULL ;

  if (path->remote && path->num_dirs > 1)
    share = path->dir[1] ;

  return (share) ;
}

BLUE_CORE_LIB BLUE_BOOL
BluePathShareCmp (BLUE_PATH *_path, BLUE_LPCTSTR share)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (path->remote && path->num_dirs > 1 && path->dir[1] != BLUE_NULL)
    {
      if (BlueCtstrcmp (path->dir[1], share) == 0)
	ret = BLUE_TRUE ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathUsername (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_LPCTSTR username ;

  username = BLUE_NULL ;

  if (path->remote)
    username = path->username ;

  return (username) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetUsername (BLUE_PATH *_path, 
					     BLUE_LPCTSTR username)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->username != BLUE_NULL)
	BlueHeapFree (path->username) ;
      path->username = BlueCtstrdup (username) ;
    }
}

BLUE_CORE_LIB BLUE_BOOL 
BluePathUsernameCmp (BLUE_PATH *_path, BLUE_LPCTSTR username) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (path->remote)
    {
      if (path->username == BLUE_NULL && username == BLUE_NULL)
	ret = BLUE_TRUE ;
      else 
	{
	  if (BlueCtstrcmp (path->username, username) == 0)
	    ret = BLUE_TRUE ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathPassword (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_LPCTSTR password ;

  password = BLUE_NULL ;

  if (path->remote)
    password = path->password ;

  return (password) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetPassword (BLUE_PATH *_path, 
					     BLUE_LPCTSTR password)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->password != BLUE_NULL)
	BlueHeapFree (path->password) ;
      path->password = BlueCtstrdup (password) ;
    }
}

BLUE_CORE_LIB BLUE_BOOL 
BluePathPasswordCmp (BLUE_PATH *_path, BLUE_LPCTSTR password) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_BOOL ret ;

  ret = BLUE_FALSE ;
  if (path->remote)
    {
      if (path->password == BLUE_NULL && password == BLUE_NULL)
	ret = BLUE_TRUE ;
      else if (path->password != BLUE_NULL && password != BLUE_NULL)
	{
	  if (BlueCtstrcmp (path->password, password) == 0)
	    ret = BLUE_TRUE ;
	}
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathDomain (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_LPCTSTR domain ;

  domain = BLUE_NULL ;

  if (path->remote)
    domain = path->domain ;

  return (domain) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetDomain (BLUE_PATH *_path, 
					   BLUE_LPCTSTR domain)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->domain != BLUE_NULL)
	BlueHeapFree (path->domain) ;
      path->domain = BlueCtstrdup (domain) ;
    }
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathDevice (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  return (path->device) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathFreeDevice (BLUE_PATH *_path) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->device) ;
  path->device = BLUE_NULL ;
}

BLUE_CORE_LIB BLUE_VOID BluePathFreeUsername (BLUE_PATH *_path) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->username) ;
  path->username = BLUE_NULL ;
}

BLUE_CORE_LIB BLUE_VOID BluePathFreePassword (BLUE_PATH *_path) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->password) ;
  path->password = BLUE_NULL ;
}

BLUE_CORE_LIB BLUE_VOID BluePathFreeDomain (BLUE_PATH *_path) 
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->domain) ;
  path->domain = BLUE_NULL ;
}

BLUE_CORE_LIB BLUE_VOID 
BluePathPromoteDirs (BLUE_PATH *_path, BLUE_UINT num_dirs)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_UINT i ;

  num_dirs = BLUE_C_MIN (num_dirs, path->num_dirs) ;

  for (i = 0 ; i < num_dirs ; i++)
    {
      BlueHeapFree (path->dir[i]) ;
      path->dir[i] = BLUE_NULL ;
    }

  for (i = num_dirs ; i < path->num_dirs ; i++)
    {
      path->dir[i - num_dirs] = path->dir[i] ;
      path->dir[i] = BLUE_NULL ;
    }

  path->num_dirs -= num_dirs ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetLocal (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->remote = BLUE_FALSE ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetRemote (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->remote = BLUE_TRUE ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetRelative (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->absolute = BLUE_FALSE ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetAbsolute (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->absolute = BLUE_TRUE ;
}

BLUE_CORE_LIB BLUE_BOOL BluePathAbsolute (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  return (path->absolute) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathSetType (BLUE_PATH *_path, BLUE_FS_TYPE fstype)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->type = fstype ;
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathFilename (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_LPCTSTR filename ;

  filename = BLUE_NULL ;

  if (path->remote)
    {
      if (path->num_dirs > 2)
	filename = path->dir[path->num_dirs - 1] ;
    }
  else
    {
      if (path->num_dirs > 0)
	filename = path->dir[path->num_dirs - 1] ;
    }
  return (filename) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathFreeFilename (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  if (path->remote)
    {
      if (path->num_dirs > 2)
	{
	  BlueHeapFree (path->dir[path->num_dirs-1]) ;
	  path->dir[path->num_dirs-1] = BLUE_NULL ;
	  path->num_dirs-- ;
	}
    }
  else
    {
      if (path->num_dirs > 0)
	{
	  BlueHeapFree (path->dir[path->num_dirs-1]) ;
	  path->dir[path->num_dirs-1] = BLUE_NULL ;
	  path->num_dirs-- ;
	}
    }
}

BLUE_CORE_LIB BLUE_INT BluePathNumDirs (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_INT ret ;

  ret = 0 ;
  if (path->remote)
    {
      if (path->num_dirs > 2)
	ret = path->num_dirs - 2 ;
    }
  else
    ret = path->num_dirs ;

  return (ret) ;
}

BLUE_CORE_LIB BLUE_LPCTSTR BluePathDir (BLUE_PATH *_path, BLUE_UINT ix)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_LPCTSTR ret ;
  BLUE_UINT jx ;

  ret = BLUE_NULL ;
  if (path->remote)
    {
      jx = path->num_dirs - 2 ;
      if (jx > 0 && ix < jx)
	ret = path->dir[ix + 2] ;
    }
  else
    {
      if (ix < path->num_dirs)
	ret = path->dir[ix] ;
    }
  return (ret) ;
}

BLUE_CORE_LIB BLUE_VOID BluePathFreeDirs (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_UINT jx ;
  BLUE_UINT ix ;

  if (path->remote)
    {
      jx = 1 ;
    }
  else
    {
      jx = 0 ;
    }

  for (ix = jx ; ix < path->num_dirs ; ix++)
    {
      BlueHeapFree (path->dir[ix]) ;
      path->dir[ix] = BLUE_NULL ;
    }

  if (path->num_dirs > jx)
    path->num_dirs = jx ;

  if (path->num_dirs == 0)
    {
      BlueHeapFree (path->dir) ;
      path->dir = BLUE_NULL ;
    }
}

BLUE_VOID BluePathDebug (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BLUE_UINT i ;
  BLUE_CHAR *test ;

  test = (BLUE_CHAR *) TSTR("NULL") ;

  for (i = 0 ; i < (5 * sizeof(BLUE_TCHAR)) ; i++)
    BlueCprintf ("Test[%d]: 0x%02x\n", i, test[i]) ;

  BlueCprintf ("Path:\n") ;
  BlueCprintf ("  Type: %d\n", path->type) ;
  BlueCprintf ("  Device: %S\n", 
	       path->device == BLUE_NULL ? TSTR("NULL") : path->device) ;
  BlueCprintf ("  Username: %S\n", 
	       path->username == BLUE_NULL ? TSTR("NULL") : path->username) ;
  BlueCprintf ("  Password: %S\n", 
	       path->password == BLUE_NULL ? TSTR("NULL") : path->password) ;
  BlueCprintf ("  Domain: %S\n", 
	       path->domain == BLUE_NULL ? TSTR("NULL") : path->domain) ;
  BlueCprintf ("  Num Dirs: %d\n", path->num_dirs) ;
  for (i = 0 ; i < path->num_dirs ; i++)
    BlueCprintf ("    %d: %S\n", i, path->dir[i]) ;
      
  BlueCprintf ("  Absolute: %s\n", path->absolute ? "yes" : "no") ;
  BlueCprintf ("  Remote: %s\n", path->remote ? "yes" : "no") ;
}  

static BLUE_HANDLE hWorkgroups ;

BLUE_CORE_LIB BLUE_VOID InitWorkgroups (BLUE_VOID)
{
  hWorkgroups = BlueQcreate() ;
}

BLUE_CORE_LIB BLUE_VOID DestroyWorkgroups (BLUE_VOID)
{
  BLUE_LPTSTR pWorkgroup ;

  for (pWorkgroup = BlueQfirst (hWorkgroups) ;
       pWorkgroup != BLUE_NULL ;
       pWorkgroup = BlueQfirst (hWorkgroups))
    RemoveWorkgroup (pWorkgroup);

  BlueQdestroy(hWorkgroups);

  hWorkgroups = BLUE_HANDLE_NULL ;
}

static BLUE_LPTSTR FindWorkgroup (BLUE_LPCTSTR workgroup)
{
  BLUE_LPTSTR pWorkgroup ;

  for (pWorkgroup = BlueQfirst (hWorkgroups) ;
       pWorkgroup != BLUE_NULL && BlueCtstrcmp (pWorkgroup, workgroup) != 0 ;
       pWorkgroup = BlueQnext (hWorkgroups, pWorkgroup)) ;

  return (pWorkgroup) ;
}

BLUE_CORE_LIB BLUE_VOID UpdateWorkgroup (BLUE_LPCTSTR workgroup)
{
  BLUE_LPTSTR pWorkgroup ;

  BlueLock (lockPath) ;
  pWorkgroup = FindWorkgroup (workgroup) ;

  if (pWorkgroup == BLUE_NULL)
    {
      pWorkgroup = BlueCtstrndup (workgroup, BLUE_MAX_PATH) ;
      BlueQenqueue (hWorkgroups, pWorkgroup) ;
    }
  BlueUnlock (lockPath) ;
}

BLUE_CORE_LIB BLUE_VOID RemoveWorkgroup (BLUE_LPCTSTR workgroup) 
{
  BLUE_LPTSTR pWorkgroup ;

  BlueLock (lockPath) ;
  pWorkgroup = FindWorkgroup (workgroup) ;

  if (pWorkgroup != BLUE_NULL)
    {
      BlueQunlink (hWorkgroups, pWorkgroup) ;
      BlueHeapFree (pWorkgroup) ;
    }
  BlueUnlock (lockPath) ;
}

BLUE_CORE_LIB BLUE_BOOL LookupWorkgroup (BLUE_LPCTSTR workgroup) 
{
  BLUE_BOOL ret ;
  BLUE_LPTSTR pWorkgroup ;

  BlueLock (lockPath) ;
  pWorkgroup = FindWorkgroup (workgroup) ;

  ret = BLUE_FALSE ;
  if (pWorkgroup != BLUE_NULL)
    ret = BLUE_TRUE ;

  BlueUnlock (lockPath) ;
  return (ret) ;
}

