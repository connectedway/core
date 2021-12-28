/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

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
  OFC_LPTSTR device ;		/**< The device name */
  OFC_LPTSTR username ;	/**< The user name  */
  OFC_LPTSTR password ;	/**< The password  */
  OFC_LPTSTR domain ;		/**< The workgroup/domain  */
  OFC_INT port ;
  OFC_UINT num_dirs ;		/**< Number of directory fields  */
  OFC_LPTSTR *dir ;		/**< Array of directory entries.
				   First is the share */
  OFC_BOOL absolute ;		/**< If the path is absolute */
  OFC_BOOL remote ;		/**< If the path is remote */
} _BLUE_PATH ;

typedef struct 
{
  OFC_LPTSTR lpDevice ;
  OFC_LPTSTR lpDesc ;
  _BLUE_PATH *map ;
  OFC_BOOL thumbnail ;
} PATH_MAP_ENTRY ;

static PATH_MAP_ENTRY BluePathMaps[OFC_MAX_MAPS] ;
static BLUE_LOCK lockPath ;

OFC_BOOL BluePathIsWild (OFC_LPCTSTR dir)
{
  OFC_LPCTSTR p ;
  OFC_BOOL wild ;

  wild = OFC_FALSE ;
  p = BlueCtstrtok (dir, TSTR("*?")) ;
  if (p != OFC_NULL && *p != TCHAR_EOS)
    wild = OFC_TRUE ;
  return (wild) ;
}

OFC_CORE_LIB OFC_VOID
BluePathUpdate (BLUE_PATH *_path, BLUE_PATH *_map) 
{
  OFC_UINT i ;

  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  _BLUE_PATH *map = (_BLUE_PATH *) _map ;

  path->type = map->type ;
  BlueHeapFree (path->device) ;
  path->device = BlueCtstrdup (map->device) ;

  if (map->username != OFC_NULL)
    {
      BlueHeapFree (path->username) ;
      path->username = BlueCtstrdup (map->username) ;
    }

  if (map->password != OFC_NULL)
    {
      BlueHeapFree (path->password) ;
      path->password = BlueCtstrdup (map->password) ;
    }

  if (map->domain != OFC_NULL)
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
			       sizeof (OFC_TCHAR *)) ;
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
				   path->num_dirs * sizeof (OFC_TCHAR *)) ;
    }
}

static OFC_BOOL special (OFC_CTCHAR ct)
{
  OFC_BOOL ret ;

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
      ret = OFC_TRUE ;
      break ;

    default:
      ret = OFC_FALSE ;
      break ;
    }
  return (ret) ;
}

static OFC_BOOL terminator (OFC_CTCHAR c, OFC_CTCHAR *terms)
{
  OFC_BOOL hit ;
  OFC_INT i ;

  if (c == '\0')
    hit = OFC_TRUE ;
  else
    {
      for (i = 0, hit = OFC_FALSE ; i < BlueCtstrlen(terms) && !hit ; i++)
	{
	  if (c == terms[i])
	    hit = OFC_TRUE ;
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

static OFC_TCHAR *ParseEscaped (OFC_CTCHAR *cursor, OFC_CTCHAR **outcursor,
                                OFC_CTCHAR *terms)
{
  OFC_LPCTSTR peek ;
  OFC_BOOL hit ;
  OFC_LPTSTR ret ;
  OFC_INT len ;
  OFC_TCHAR *p ;
  OFC_TCHAR *outstr ;

  outstr = OFC_NULL ;
  *outcursor = cursor ;

  if (cursor == OFC_NULL || terms == OFC_NULL)
    ret = OFC_NULL ;
  else
    {
      /*
       * First, find the length
       */
      for (peek = cursor, len = 0, hit = OFC_FALSE ; !hit ; )
	{
	  OFC_INT i ;
	  OFC_CHAR c ;

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
		  hit = OFC_TRUE ;
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
	  outstr = BlueHeapMalloc (sizeof (OFC_TCHAR) * (len + 1)) ;
	  p = outstr ;

	  for (peek = cursor, hit = OFC_FALSE ; !hit ;)
	    {
	      OFC_INT i ;
	      OFC_CHAR c ;

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
		      hit = OFC_TRUE ;
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

OFC_CORE_LIB BLUE_PATH *
BluePathInitPath (OFC_VOID)
{
  _BLUE_PATH *path ;

  path = BlueHeapMalloc (sizeof (_BLUE_PATH)) ;
  path->type = BLUE_FS_FILE ;
  path->device = OFC_NULL ;
  path->username = OFC_NULL ;
  path->password = OFC_NULL ;
  path->domain = OFC_NULL ;
  path->port = 0 ;
  path->num_dirs = 0 ;
  path->dir = OFC_NULL ;
  path->absolute = OFC_FALSE ;
  path->remote = OFC_FALSE ;

  return (path) ;
}

static OFC_VOID BluePathUpdateType (BLUE_PATH *path)
{
#if defined(OFC_FS_CIFS)
  if ((BluePathType(path) == BLUE_FS_UNKNOWN ||
       BluePathType(path) == BLUE_FS_FILE) && BluePathRemote(path))
    BluePathSetType (path, BLUE_FS_CIFS) ;
#endif

  if (BluePathType(path) == BLUE_FS_UNKNOWN ||
      BluePathType(path) == BLUE_FS_FILE)
    {
#if defined(OFC_FS_WIN32)
      BluePathSetType (path, BLUE_FS_WIN32) ;
#elif defined(OFC_FS_WINCE)
      BluePathSetType (path, BLUE_FS_WINCE) ;
#elif defined(OFC_FS_DARWIN)
      BluePathSetType (path, BLUE_FS_DARWIN) ;
#elif defined(OFC_FS_LINUX)
      BluePathSetType (path, BLUE_FS_LINUX) ;
#elif defined(OFC_FS_ANDROID)
      BluePathSetType (path, BLUE_FS_ANDROID) ;
#elif defined(OFC_FS_FILEX)
      BluePathSetType (path, BLUE_FS_FILEX) ;
#endif
    }
}

OFC_CORE_LIB BLUE_PATH *
BluePathCreateW (OFC_LPCTSTR lpFileName)
{
  _BLUE_PATH *path ;
  OFC_LPCTSTR cursor ;
  OFC_LPCTSTR p ;
  OFC_TCHAR *portw ;
  OFC_CHAR *porta ;
  OFC_LPTSTR dir ;
  OFC_TCHAR *credentials ;
  OFC_BOOL wild ;

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
	path->remote = OFC_TRUE ;
      /*
       * We expect a device, colon and two separators for a uri
       * We expect a device, colon and one separator for an absolute DOS path
       * We expect a device, colong and no separators for a relative DOS path
       */
      if ((cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) &&
	  (cursor[1] == TCHAR_SLASH || cursor[1] == TCHAR_BACKSLASH))
	{
	  cursor += 2 ;
	  path->absolute = OFC_TRUE ;
	}
      else if (cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH)
	{
	  cursor ++ ;
	  path->absolute = OFC_TRUE ;
	}
    }
  else if ((cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) &&
	   (cursor[1] == TCHAR_SLASH || cursor[1] == TCHAR_BACKSLASH))
    {
      /* UNC path */
      cursor += 2 ;
      path->remote = OFC_TRUE ;
      path->absolute = OFC_TRUE ;
    }
  else if (cursor[0] == TCHAR_SLASH || cursor[0] == TCHAR_BACKSLASH) 
    {
      cursor++ ;
      path->absolute = OFC_TRUE ;
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

      if (dir != OFC_NULL)
	{
	  wild = BluePathIsWild (dir) ;

	  if (BlueCtstrcmp (dir, TSTR("..")) == 0)
	    {
	      if (path->num_dirs > 0)
		{
		  path->num_dirs-- ;
		  BlueHeapFree (path->dir[path->num_dirs]) ;
		  path->dir = BlueHeapRealloc (path->dir,
                                       sizeof (OFC_LPCTSTR *) *
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
                                       sizeof (OFC_LPCTSTR *) *
                                       (path->num_dirs + 1)) ;
	      path->dir[path->num_dirs] = dir ;
	      path->num_dirs++ ;
	    }

	  if ((*cursor == TCHAR(':')) && path->remote && (path->num_dirs == 1))
	    {
	      cursor++ ;
	      portw = ParseEscaped (cursor, &cursor, TSTR("\\/")) ;
	      porta = BlueCtstr2cstr (portw) ;
	      path->port = (OFC_INT) BlueCstrtoul (porta, OFC_NULL, 10) ;
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

OFC_CORE_LIB BLUE_PATH *
BluePathCreateA (OFC_LPCSTR lpFileName)
{
  _BLUE_PATH *ret ;
  OFC_TCHAR *lptFileName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  ret = BluePathCreateW (lptFileName) ;
  BlueHeapFree (lptFileName) ;
  return ((BLUE_PATH *) ret) ;
}

OFC_CORE_LIB OFC_VOID
BluePathDelete (BLUE_PATH *_path)
{
  OFC_UINT i ;
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

static OFC_SIZET
BluePathOutChar (OFC_TCHAR c, OFC_LPTSTR *dest, OFC_SIZET *rem)
{
  if (*rem > 0)
    {
      (*rem)-- ;
      *(*dest)++ = c ;
    }
  return (1) ;
}


static OFC_SIZET
BluePathOutEscaped (OFC_LPCTSTR str, OFC_LPTSTR *filename, OFC_SIZET *rem)
{
  OFC_LPCTSTR p ;
  OFC_SIZET len ;

  len = 0 ;
  for (p = str ; *p != TCHAR_EOS ; p++)
    {
      if (special(*p))
	{
	  OFC_TCHAR c ;
	  
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

static OFC_SIZET
BluePathOutEscapedLC (OFC_LPCTSTR str, OFC_LPTSTR *filename, OFC_SIZET *rem)
{
  OFC_LPCTSTR p ;
  OFC_SIZET len ;

  len = 0 ;
  for (p = str ; *p != TCHAR_EOS ; p++)
    {
      if (special(*p))
	{
	  OFC_TCHAR c ;
	  
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

static OFC_SIZET
BluePathOutStr (OFC_LPCTSTR str, OFC_LPTSTR *filename, OFC_SIZET *rem)
{
  OFC_LPCTSTR p ;
  OFC_SIZET len ;

  len = 0 ;
  for (p = str ; *p != TCHAR_EOS ; p++)
    {
      len += BluePathOutChar (*p, filename, rem) ;
    }
    
  return (len) ;
}

OFC_CORE_LIB OFC_SIZET
BluePathPrintW (BLUE_PATH *_path, OFC_LPTSTR *filename, OFC_SIZET *rem)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_TCHAR delimeter ;
  OFC_SIZET len ;
  OFC_UINT i ;

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
#if defined(OFC_FS_WIN32)
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

  if (path->device != OFC_NULL)
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
      if (path->username != OFC_NULL)
	{
	  len += BluePathOutEscaped (path->username, filename, rem) ;

	  if (path->password != OFC_NULL || path->domain != OFC_NULL)
	    {
	      len += BluePathOutChar (TCHAR_COLON, filename, rem) ;

	      if (path->password != OFC_NULL &&
		  path->password[0] != TCHAR_EOS)
		{
		  len += BluePathOutEscaped (path->password, filename, rem) ;
		  
		  if (path->domain != OFC_NULL)
		    {
		      len += BluePathOutChar (TCHAR_COLON, filename, rem) ;
		    }
		}

	      if (path->domain != OFC_NULL)
		{
		  len += BluePathOutEscaped (path->domain, filename, rem) ;
		}
	    }
	  len += BluePathOutChar (TCHAR_AMP, filename, rem) ;
	}
    }

  for (i = 0 ; i < path->num_dirs ; i++)
    {
      if (path->dir[i] != OFC_NULL &&
	  BlueCtstrcmp(path->dir[i], TSTR("")) != 0)
	{
	  len += BluePathOutStr (path->dir[i], filename, rem) ;

	  if (path->remote && (i == 0) && (path->port != 0))
	    {
	      OFC_SIZET count ;
	      OFC_CHAR *porta ;
	      OFC_TCHAR *portw ;

	      count = 0 ;
	      count = BlueCsnprintf (OFC_NULL, count, "%d", path->port) ;
	      porta = BlueHeapMalloc ((count+1)*sizeof(OFC_CHAR)) ;
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

OFC_CORE_LIB OFC_SIZET
BluePathPrintA (BLUE_PATH *_path, OFC_LPSTR *filename, OFC_SIZET *rem)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_SIZET ret ;
  OFC_SIZET orig_rem ;
  OFC_TCHAR *tfilename ;
  OFC_TCHAR *ptfilename ;
  OFC_CHAR *pfilename ;
  OFC_INT i ;

  orig_rem = *rem ;
  tfilename = BlueHeapMalloc (*rem * sizeof (OFC_TCHAR)) ;
  ptfilename = tfilename ;
  ret = BluePathPrintW (path, &ptfilename, rem) ;
  if (filename != OFC_NULL)
    {
      pfilename = *filename ;
      for (i = 0 ; i < BLUE_C_MIN (orig_rem, ret + 1) ; i++)
	*pfilename++ = (OFC_CHAR) tfilename[i] ;
    }
  BlueHeapFree (tfilename) ;
  return (ret) ;
}

OFC_CORE_LIB BLUE_FS_TYPE BluePathType (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  return (path->type) ;
}

/*
 * Inner workings of BluePathMapW.  Returns the resulting path
 * A map is a device (gen:/temp) or a server name (//gen/temp)
 */
OFC_CORE_LIB BLUE_PATH *BlueMapPath (OFC_LPCTSTR lpFileName,
                                     OFC_LPTSTR *lppMappedName)
{
  BLUE_PATH *path ;
  BLUE_PATH *map ;
  OFC_SIZET len ;
  OFC_LPTSTR cursor ;
#if defined(AUTHENTICATE)
  OFC_LPCTSTR server ;
  OFC_BOOL updated ;
  OFC_LPCTSTR previous_server ;
#endif
  
  path = BluePathCreateW (lpFileName) ;

#if defined(AUTHENTICATE)
  previous_server = OFC_NULL ;
  do
    {
      updated = OFC_FALSE ;
      server = BluePathDevice(path) ;

      if (server == OFC_NULL)
	server = BluePathServer(path) ;

      if (server != OFC_NULL)
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
	  if (previous_server == OFC_NULL ||
	      BlueCtstrcmp (server, previous_server) != 0)
	    {
	      previous_server = server ;
	      map = BluePathMapDeviceW (server) ;

	      if (map != OFC_NULL)
		{
		  BluePathUpdate (path, map) ;
		  updated = OFC_TRUE ;
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

  if (lppMappedName != OFC_NULL)
    {
      len = 0 ;
      len = BluePathPrintW (path, OFC_NULL, &len) + 1 ;

      *lppMappedName = BlueHeapMalloc (len * sizeof(OFC_TCHAR)) ;

      cursor = *lppMappedName ;
      BluePathPrintW (path, &cursor, &len) ;
    }

  return (path) ;
}
  
/*
 * A map is a device (i.e. gen:/temp) or a server name: (i.e. //gen/temp
 */
OFC_CORE_LIB OFC_VOID
BluePathMapW (OFC_LPCTSTR lpFileName, OFC_LPTSTR *lppMappedName,
              BLUE_FS_TYPE *filesystem)
{
  BLUE_PATH *path ;
  
  path = BlueMapPath (lpFileName, lppMappedName) ;

  if (filesystem != OFC_NULL)
    *filesystem = BluePathType(path) ;
  BluePathDelete (path) ;
}

OFC_CORE_LIB OFC_VOID
BluePathMapA (OFC_LPCSTR lpFileName, OFC_LPSTR *lppMappedName,
              BLUE_FS_TYPE *filesystem)
{
  OFC_TCHAR *lptFileName ;
  OFC_TCHAR *lptMappedName ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  lptMappedName = OFC_NULL ;
  BluePathMapW (lptFileName, &lptMappedName, filesystem) ;
  if (lppMappedName != OFC_NULL)
    *lppMappedName = BlueCtstr2cstr (lptMappedName) ;
  BlueHeapFree (lptFileName) ;
  BlueHeapFree (lptMappedName) ;
}

OFC_CORE_LIB OFC_VOID
BluePathInit (OFC_VOID)
{
  OFC_INT i ;

  lockPath = BlueLockInit () ;

  for (i = 0 ; i < OFC_MAX_MAPS ; i++)
    {
      BluePathMaps[i].lpDevice = OFC_NULL ;
      BluePathMaps[i].lpDesc = OFC_NULL ;
      BluePathMaps[i].map = OFC_NULL ;
      BluePathMaps[i].thumbnail = OFC_FALSE ;
    }
}      

OFC_CORE_LIB OFC_VOID
BluePathDestroy (OFC_VOID)
{
  OFC_INT i ;

  for (i = 0 ; i < OFC_MAX_MAPS ; i++)
    {
      if (BluePathMaps[i].lpDevice != OFC_NULL)
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
OFC_CORE_LIB OFC_BOOL
BluePathAddMapW (OFC_LPCTSTR lpDevice, OFC_LPCTSTR lpDesc,
                 BLUE_PATH *_map, BLUE_FS_TYPE fsType, OFC_BOOL thumbnail)
{
  _BLUE_PATH *map = (_BLUE_PATH *) _map ;
  OFC_INT i ;
  OFC_BOOL ret ;
  PATH_MAP_ENTRY *free ;
  OFC_TCHAR *lc ;
  /*
   * Search for a free entry and make sure the device is not already mapped
   */
  ret = OFC_TRUE ;
  free = OFC_NULL ;

  BlueLock (lockPath) ;

  for (i = 0 ; i < OFC_MAX_MAPS && ret == OFC_TRUE ; i++)
    {
      if (BluePathMaps[i].lpDevice == OFC_NULL)
        {
          if (free == OFC_NULL)
            free = &BluePathMaps[i] ;
        }
      else
        {
          if (BlueCtstrcasecmp (BluePathMaps[i].lpDevice, lpDevice) == 0)
            {
              ret = OFC_FALSE ;
            }
        }
    }

  if (ret == OFC_TRUE)
    {
      if (free == OFC_NULL)
	{
	  ret = OFC_FALSE ;
	}
      else
	{
	  if (lpDevice == OFC_NULL)
	    ret = OFC_FALSE ;
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
  
OFC_CORE_LIB OFC_BOOL
BluePathAddMapA (OFC_LPCSTR lpDevice, OFC_LPCSTR lpDesc,
                 BLUE_PATH *_map, BLUE_FS_TYPE fsType, OFC_BOOL thumbnail)
{
  OFC_BOOL ret ;
  OFC_TCHAR *lptDevice ;
  OFC_TCHAR *lptDesc ;

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
OFC_CORE_LIB OFC_VOID
BluePathDeleteMapW (OFC_LPCTSTR lpDevice)
{
  OFC_INT i ;
  PATH_MAP_ENTRY *pathEntry ;

  /*
   * Search for a free entry and make sure the device is not already mapped
   */
  pathEntry = OFC_NULL ;

  BlueLock (lockPath) ;

  for (i = 0 ; i < OFC_MAX_MAPS && pathEntry == OFC_NULL ; i++)
    {
      if (BluePathMaps[i].lpDevice != OFC_NULL &&
          BlueCtstrcasecmp (BluePathMaps[i].lpDevice, lpDevice) == 0)
        pathEntry = &BluePathMaps[i] ;
    }

  if (pathEntry != OFC_NULL)
    {
      BlueHeapFree (pathEntry->lpDevice) ;
      BlueHeapFree (pathEntry->lpDesc) ;
      BluePathDelete (pathEntry->map) ;
      pathEntry->lpDevice = OFC_NULL ;
      pathEntry->map = OFC_NULL ;
    }

  BlueUnlock (lockPath) ;
}

OFC_CORE_LIB OFC_VOID
BluePathDeleteMapA (OFC_LPCSTR lpDevice)
{
  OFC_TCHAR *lptDevice ;

  lptDevice = BlueCcstr2tstr (lpDevice) ;
  BluePathDeleteMapW (lptDevice) ;
  BlueHeapFree (lptDevice) ;
}

OFC_CORE_LIB OFC_VOID
BluePathGetMapW (OFC_INT idx, OFC_LPCTSTR *lpDevice,
                 OFC_LPCTSTR *lpDesc, BLUE_PATH **map,
                 OFC_BOOL *thumbnail)
{
  BlueLock (lockPath) ;

  *lpDevice = BluePathMaps[idx].lpDevice ;
  *lpDesc = BluePathMaps[idx].lpDesc ;
  *map = BluePathMaps[idx].map ;
  *thumbnail = BluePathMaps[idx].thumbnail ;
  BlueUnlock (lockPath) ;
}
  
OFC_CORE_LIB BLUE_PATH *
BluePathMapDeviceW (OFC_LPCTSTR lpDevice)
{
  OFC_INT i ;
  PATH_MAP_ENTRY *pathEntry ;
  BLUE_PATH *map ;
  OFC_CTCHAR *p ;
  OFC_TCHAR *lc ;
  OFC_SIZET len ;
  OFC_LPTSTR tstrDevice ;

  /*
   * Search for a free entry and make sure the device is not already mapped
   */
  map = OFC_NULL ;
  if (lpDevice != OFC_NULL)
    {
      /*
       * Peel off just the device portion.  Normal users pass in the
       * device only anyway but this allows the routine to be more general.
       * We prefer sending in a full path when determining if the path
       * is served by one of our maps
       */
      p = BlueCtstrtok (lpDevice, TSTR(":")) ;
      len = ((OFC_ULONG_PTR) p - (OFC_ULONG_PTR) lpDevice) /
            sizeof (OFC_TCHAR) ;
      tstrDevice = BlueHeapMalloc ((len + 1) * sizeof (OFC_TCHAR)) ;

      BlueCtstrncpy (tstrDevice, lpDevice, len) ;
      tstrDevice[len] = TCHAR_EOS ;
	
      for (lc = tstrDevice ; *lc != TCHAR_EOS ; lc++)
	*lc = BLUE_C_TOLOWER(*lc) ;

      pathEntry = OFC_NULL ;

      for (i = 0 ; i < OFC_MAX_MAPS && pathEntry == OFC_NULL ; i++)
	{
	  if (BluePathMaps[i].lpDevice != OFC_NULL &&
	      BlueCtstrcasecmp (BluePathMaps[i].lpDevice, tstrDevice) == 0)
	    {
	      pathEntry = &BluePathMaps[i] ;
	    }
	}

      BlueHeapFree (tstrDevice) ;
      if (pathEntry != OFC_NULL)
	map = pathEntry->map ;	
    }
  return (map) ;
}

OFC_CORE_LIB BLUE_PATH *
BluePathMapDeviceA (OFC_LPCSTR lpDevice)
{
  _BLUE_PATH *ret ;
  OFC_TCHAR *lptDevice ;

  lptDevice = BlueCcstr2tstr (lpDevice) ;
  ret = BluePathMapDeviceW (lptDevice) ;
  BlueHeapFree (lptDevice) ;
  return ((BLUE_PATH *) ret) ;
}

OFC_CORE_LIB OFC_SIZET BluePathMakeURLW (OFC_LPTSTR *filename,
                                         OFC_SIZET *rem,
                                         OFC_LPCTSTR username,
                                         OFC_LPCTSTR password,
                                         OFC_LPCTSTR domain,
                                         OFC_LPCTSTR server,
                                         OFC_LPCTSTR share,
                                         OFC_LPCTSTR path,
                                         OFC_LPCTSTR file)
  
{
  OFC_TCHAR delimeter ;
  OFC_SIZET len ;

  delimeter = TCHAR_BACKSLASH ;
  /*
   * Need to do an update remainder kind of thing
   */
  len = 0 ;

  len += BluePathOutChar (delimeter, filename, rem) ;
  len += BluePathOutChar (delimeter, filename, rem) ;

  if (username != OFC_NULL)
    {
      len += BluePathOutEscaped (username, filename, rem) ;

      if (password != OFC_NULL || domain != OFC_NULL)
	{
	  len += BluePathOutChar (TCHAR_COLON, filename, rem) ;

	  if (password != OFC_NULL && password[0] != TCHAR_EOS)
	    {
	      len += BluePathOutEscaped (password, filename, rem) ;
		  
	      if (domain != OFC_NULL)
		{
		  len += BluePathOutChar (TCHAR_COLON, filename, rem) ;
		}
	    }

	  if (domain != OFC_NULL)
	    {
	      len += BluePathOutEscaped (domain, filename, rem) ;
	    }
	}
      len += BluePathOutChar (TCHAR_AMP, filename, rem) ;
    }

  if (server != OFC_NULL)
    {
      len += BluePathOutStr (server, filename, rem) ;
      len += BluePathOutChar (delimeter, filename, rem) ;
    }

  if (share != OFC_NULL)
    {
      len += BluePathOutStr (share, filename, rem) ;
      len += BluePathOutChar (delimeter, filename, rem) ;
    }

  if (path != OFC_NULL)
    {
      len += BluePathOutStr (path, filename, rem) ;
      len += BluePathOutChar (delimeter, filename, rem) ;
    }

  if (file != OFC_NULL)
    {
      len += BluePathOutStr (file, filename, rem) ;
    }

  BluePathOutChar (TCHAR_EOS, filename, rem) ;
  return (len) ;
}

OFC_CORE_LIB OFC_SIZET BluePathMakeURLA (OFC_LPSTR *filename,
                                         OFC_SIZET *rem,
                                         OFC_LPCSTR username,
                                         OFC_LPCSTR password,
                                         OFC_LPCSTR domain,
                                         OFC_LPCSTR server,
                                         OFC_LPCSTR share,
                                         OFC_LPCSTR path,
                                         OFC_LPCSTR file)
{
  OFC_LPTSTR tfilename ;
  OFC_LPTSTR tcursor ;
  OFC_LPTSTR tusername ;
  OFC_LPTSTR tpassword ;
  OFC_LPTSTR tdomain ;
  OFC_LPTSTR tserver ;
  OFC_LPTSTR tshare ;
  OFC_LPTSTR tpath ;
  OFC_LPTSTR tfile ;
  OFC_SIZET len ;
  OFC_SIZET orig_rem ;
  OFC_LPSTR pfilename ;
  OFC_INT i ;

  tusername = BlueCcstr2tstr (username) ;
  tpassword = BlueCcstr2tstr (password) ;
  tdomain = BlueCcstr2tstr (domain) ;
  tserver = BlueCcstr2tstr (server) ;
  tshare = BlueCcstr2tstr (share) ;
  tpath = BlueCcstr2tstr (path) ;
  tfile = BlueCcstr2tstr (file) ;

  orig_rem = *rem ;
  tfilename = BlueHeapMalloc ((*rem+1) * sizeof (OFC_TCHAR)) ;
  tcursor = tfilename ;

  len = BluePathMakeURLW (&tcursor, rem, tusername, tpassword, tdomain,
			  tserver, tshare, tpath, tfile) ;

  pfilename = *filename ;
  for (i = 0 ; i < BLUE_C_MIN (orig_rem, len + 1) ; i++)
    *pfilename++ = (OFC_CHAR) tfilename[i] ;

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
OFC_CORE_LIB OFC_VOID
BluePathUpdateCredentialsW (OFC_LPCTSTR filename, OFC_LPCTSTR username,
                            OFC_LPCTSTR password, OFC_LPCTSTR domain)
{
  BLUE_PATH *path ;
  BLUE_PATH *map ;
  _BLUE_PATH *_map ;
  OFC_LPCTSTR server ;

  path = BluePathCreateW (filename) ;

  server = BluePathServer(path) ;

  if (server != OFC_NULL)
    {
      map = BluePathMapDeviceW (BluePathServer(path)) ;

      if (map == OFC_NULL)
	{
	  /*
	   * We just want the path to contain a server (and authentication)
	   */
	  BluePathFreeDirs(path) ;
	  /*
	   * Add server to data base
	   */
	  BluePathAddMapW (BluePathServer(path), TSTR("Server"),
                       path, BLUE_FS_CIFS, OFC_TRUE) ;
	  map = path ;
	}

      BlueLock (lockPath) ;

      _map = (_BLUE_PATH *) map ;

      if (_map->username != OFC_NULL)
	BlueHeapFree (_map->username) ;
      _map->username = BlueCtstrdup (username) ;
      if (_map->password != OFC_NULL)
	{
	  BlueCmemset (_map->password, '\0', 
		       BlueCtstrlen (_map->password) * sizeof (OFC_TCHAR)) ;
	  BlueHeapFree (_map->password) ;
	}
      _map->password = BlueCtstrdup (password) ;
      if (_map->domain != OFC_NULL)
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
OFC_CORE_LIB OFC_VOID
BluePathUpdateCredentialsA (OFC_LPCSTR filename, OFC_LPCSTR username,
                            OFC_LPCSTR password, OFC_LPCSTR domain)
{
  OFC_TCHAR *tusername ;
  OFC_TCHAR *tpassword ;
  OFC_TCHAR *tdomain ;
  OFC_TCHAR *tfilename ;

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

OFC_CORE_LIB OFC_VOID
BluePathGetRootW (OFC_CTCHAR *lpFileName, OFC_TCHAR **lpRootName,
                  BLUE_FS_TYPE *filesystem)
{
  OFC_TCHAR *lpMappedPath ;
  OFC_TCHAR *lpMappedName ;
  _BLUE_PATH *rootpath ;
  _BLUE_PATH *path ;
  OFC_SIZET len ;

  BluePathMapW (lpFileName, &lpMappedName, filesystem) ;
  path = BluePathCreateW (lpMappedName) ;

  if (path->remote)
    {
      rootpath = BluePathCreateW (TSTR("\\")) ;
      rootpath->absolute = OFC_TRUE ;
      rootpath->device = BlueCtstrdup (path->device) ;
      /*
       * Then the share is part of the root
       */
      rootpath->remote = OFC_TRUE ;
      rootpath->port = path->port ;
      rootpath->username = BlueCtstrdup (path->username) ;
      rootpath->password = BlueCtstrdup (path->password) ;
      rootpath->domain = BlueCtstrdup (path->domain) ;
      if (path->num_dirs > 1)
	{
	  rootpath->num_dirs = 2 ;
	  rootpath->dir = BlueHeapMalloc (sizeof (OFC_LPCTSTR *) *
                                      rootpath->num_dirs) ;
	  rootpath->dir[0] = BlueCtstrdup (path->dir[0]) ;
	  rootpath->dir[1] = BlueCtstrdup (path->dir[1]) ;
	}
    }
  else
    rootpath = BluePathCreateW (lpMappedName) ;

  len = 0 ;
  len = BluePathPrintW (rootpath, OFC_NULL, &len) ;

  if (rootpath->type == BLUE_FS_WIN32)
    /* Terminating slash */
    len++ ;
  /* EOS */
  len++ ;
  *lpRootName = BlueHeapMalloc (len * sizeof (OFC_TCHAR)) ;
  lpMappedPath = *lpRootName ;
  len = BluePathPrintW (rootpath, &lpMappedPath, &len) ;
  if (rootpath->type == BLUE_FS_WIN32)
    (*lpRootName)[len++] = TCHAR_BACKSLASH ;
  (*lpRootName)[len] = TCHAR_EOS ;
  BluePathDelete (rootpath) ;
  BluePathDelete (path) ;
  BlueHeapFree (lpMappedName) ;
}

OFC_CORE_LIB OFC_VOID
BluePathGetRootA (OFC_CCHAR *lpFileName, OFC_CHAR **lpRoot,
                  BLUE_FS_TYPE *filesystem)
{
  OFC_TCHAR *lptFileName ;
  OFC_TCHAR *lptRoot ;

  lptFileName = BlueCcstr2tstr (lpFileName) ;
  BluePathGetRootW (lptFileName, &lptRoot, filesystem) ;
  *lpRoot = BlueCtstr2cstr (lptRoot) ;
  BlueHeapFree (lptFileName) ;
  BlueHeapFree (lptRoot) ;
}

OFC_CORE_LIB OFC_BOOL BluePathRemote (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (path->remote)
    ret = OFC_TRUE ;
  return (ret) ;
}

OFC_CORE_LIB OFC_INT BluePathPort (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_INT port ;

  port = path->port ;

  return (port) ;
}

OFC_CORE_LIB OFC_VOID BluePathSetPort (BLUE_PATH *_path, OFC_INT port)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  path->port = port ;
}

OFC_CORE_LIB OFC_LPCTSTR BluePathServer (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_LPCTSTR server ;

  server = OFC_NULL ;

  if (path->remote && path->num_dirs > 0)
    server = path->dir[0] ;

  return (server) ;
}

OFC_CORE_LIB OFC_VOID BluePathFreeServer (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote && path->num_dirs > 0)
    {
      BlueHeapFree (path->dir[0]) ;
      path->dir[0] = OFC_NULL ;
    }
}

OFC_CORE_LIB OFC_VOID
BluePathSetServer (BLUE_PATH *_path, OFC_LPTSTR server)
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
						   sizeof (OFC_LPCSTR))) ;
	}
      path->dir[0] = (OFC_LPTSTR) server ;
    }
  else
    BlueHeapFree (server) ;
}

OFC_CORE_LIB OFC_VOID
BluePathSetShare (BLUE_PATH *_path, OFC_LPCTSTR share)
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
						   sizeof (OFC_LPCSTR))) ;
	}
      path->dir[1] = BlueCtstrdup (share) ;
    }
}

OFC_CORE_LIB OFC_VOID
BluePathSetFilename (BLUE_PATH *_path, OFC_LPCTSTR filename)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_UINT prefix ;

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
					       sizeof (OFC_LPCSTR))) ;
    }
  path->dir[path->num_dirs - 1] = BlueCtstrdup (filename) ;
}

OFC_CORE_LIB OFC_BOOL
BluePathServerCmp (BLUE_PATH *_path, OFC_LPCTSTR server)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (path->remote && path->num_dirs > 0 && path->dir[0] != OFC_NULL)
    {
      if (BlueCtstrcmp (path->dir[0], server) == 0)
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_BOOL
BluePathPortCmp (BLUE_PATH *_path, OFC_UINT16 port)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (path->remote && (path->port == port))
    ret = OFC_TRUE ;
  return (ret) ;
}

OFC_CORE_LIB OFC_LPCTSTR BluePathShare (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_LPCTSTR share ;

  share = OFC_NULL ;

  if (path->remote && path->num_dirs > 1)
    share = path->dir[1] ;

  return (share) ;
}

OFC_CORE_LIB OFC_BOOL
BluePathShareCmp (BLUE_PATH *_path, OFC_LPCTSTR share)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (path->remote && path->num_dirs > 1 && path->dir[1] != OFC_NULL)
    {
      if (BlueCtstrcmp (path->dir[1], share) == 0)
	ret = OFC_TRUE ;
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_LPCTSTR BluePathUsername (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_LPCTSTR username ;

  username = OFC_NULL ;

  if (path->remote)
    username = path->username ;

  return (username) ;
}

OFC_CORE_LIB OFC_VOID BluePathSetUsername (BLUE_PATH *_path,
                                           OFC_LPCTSTR username)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->username != OFC_NULL)
	BlueHeapFree (path->username) ;
      path->username = BlueCtstrdup (username) ;
    }
}

OFC_CORE_LIB OFC_BOOL
BluePathUsernameCmp (BLUE_PATH *_path, OFC_LPCTSTR username)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (path->remote)
    {
      if (path->username == OFC_NULL && username == OFC_NULL)
	ret = OFC_TRUE ;
      else 
	{
	  if (BlueCtstrcmp (path->username, username) == 0)
	    ret = OFC_TRUE ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_LPCTSTR BluePathPassword (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_LPCTSTR password ;

  password = OFC_NULL ;

  if (path->remote)
    password = path->password ;

  return (password) ;
}

OFC_CORE_LIB OFC_VOID BluePathSetPassword (BLUE_PATH *_path,
                                           OFC_LPCTSTR password)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->password != OFC_NULL)
	BlueHeapFree (path->password) ;
      path->password = BlueCtstrdup (password) ;
    }
}

OFC_CORE_LIB OFC_BOOL
BluePathPasswordCmp (BLUE_PATH *_path, OFC_LPCTSTR password)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_BOOL ret ;

  ret = OFC_FALSE ;
  if (path->remote)
    {
      if (path->password == OFC_NULL && password == OFC_NULL)
	ret = OFC_TRUE ;
      else if (path->password != OFC_NULL && password != OFC_NULL)
	{
	  if (BlueCtstrcmp (path->password, password) == 0)
	    ret = OFC_TRUE ;
	}
    }
  return (ret) ;
}

OFC_CORE_LIB OFC_LPCTSTR BluePathDomain (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_LPCTSTR domain ;

  domain = OFC_NULL ;

  if (path->remote)
    domain = path->domain ;

  return (domain) ;
}

OFC_CORE_LIB OFC_VOID BluePathSetDomain (BLUE_PATH *_path,
                                         OFC_LPCTSTR domain)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;

  if (path->remote)
    {
      if (path->domain != OFC_NULL)
	BlueHeapFree (path->domain) ;
      path->domain = BlueCtstrdup (domain) ;
    }
}

OFC_CORE_LIB OFC_LPCTSTR BluePathDevice (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  return (path->device) ;
}

OFC_CORE_LIB OFC_VOID BluePathFreeDevice (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->device) ;
  path->device = OFC_NULL ;
}

OFC_CORE_LIB OFC_VOID BluePathFreeUsername (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->username) ;
  path->username = OFC_NULL ;
}

OFC_CORE_LIB OFC_VOID BluePathFreePassword (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->password) ;
  path->password = OFC_NULL ;
}

OFC_CORE_LIB OFC_VOID BluePathFreeDomain (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  BlueHeapFree (path->domain) ;
  path->domain = OFC_NULL ;
}

OFC_CORE_LIB OFC_VOID
BluePathPromoteDirs (BLUE_PATH *_path, OFC_UINT num_dirs)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_UINT i ;

  num_dirs = BLUE_C_MIN (num_dirs, path->num_dirs) ;

  for (i = 0 ; i < num_dirs ; i++)
    {
      BlueHeapFree (path->dir[i]) ;
      path->dir[i] = OFC_NULL ;
    }

  for (i = num_dirs ; i < path->num_dirs ; i++)
    {
      path->dir[i - num_dirs] = path->dir[i] ;
      path->dir[i] = OFC_NULL ;
    }

  path->num_dirs -= num_dirs ;
}

OFC_CORE_LIB OFC_VOID BluePathSetLocal (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->remote = OFC_FALSE ;
}

OFC_CORE_LIB OFC_VOID BluePathSetRemote (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->remote = OFC_TRUE ;
}

OFC_CORE_LIB OFC_VOID BluePathSetRelative (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->absolute = OFC_FALSE ;
}

OFC_CORE_LIB OFC_VOID BluePathSetAbsolute (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->absolute = OFC_TRUE ;
}

OFC_CORE_LIB OFC_BOOL BluePathAbsolute (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  return (path->absolute) ;
}

OFC_CORE_LIB OFC_VOID BluePathSetType (BLUE_PATH *_path, BLUE_FS_TYPE fstype)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  path->type = fstype ;
}

OFC_CORE_LIB OFC_LPCTSTR BluePathFilename (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_LPCTSTR filename ;

  filename = OFC_NULL ;

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

OFC_CORE_LIB OFC_VOID BluePathFreeFilename (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  if (path->remote)
    {
      if (path->num_dirs > 2)
	{
	  BlueHeapFree (path->dir[path->num_dirs-1]) ;
	  path->dir[path->num_dirs-1] = OFC_NULL ;
	  path->num_dirs-- ;
	}
    }
  else
    {
      if (path->num_dirs > 0)
	{
	  BlueHeapFree (path->dir[path->num_dirs-1]) ;
	  path->dir[path->num_dirs-1] = OFC_NULL ;
	  path->num_dirs-- ;
	}
    }
}

OFC_CORE_LIB OFC_INT BluePathNumDirs (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_INT ret ;

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

OFC_CORE_LIB OFC_LPCTSTR BluePathDir (BLUE_PATH *_path, OFC_UINT ix)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_LPCTSTR ret ;
  OFC_UINT jx ;

  ret = OFC_NULL ;
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

OFC_CORE_LIB OFC_VOID BluePathFreeDirs (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_UINT jx ;
  OFC_UINT ix ;

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
      path->dir[ix] = OFC_NULL ;
    }

  if (path->num_dirs > jx)
    path->num_dirs = jx ;

  if (path->num_dirs == 0)
    {
      BlueHeapFree (path->dir) ;
      path->dir = OFC_NULL ;
    }
}

OFC_VOID BluePathDebug (BLUE_PATH *_path)
{
  _BLUE_PATH *path = (_BLUE_PATH *) _path ;
  OFC_UINT i ;
  OFC_CHAR *test ;

  test = (OFC_CHAR *) TSTR("NULL") ;

  for (i = 0 ; i < (5 * sizeof(OFC_TCHAR)) ; i++)
    BlueCprintf ("Test[%d]: 0x%02x\n", i, test[i]) ;

  BlueCprintf ("Path:\n") ;
  BlueCprintf ("  Type: %d\n", path->type) ;
  BlueCprintf ("  Device: %S\n",
               path->device == OFC_NULL ? TSTR("NULL") : path->device) ;
  BlueCprintf ("  Username: %S\n",
               path->username == OFC_NULL ? TSTR("NULL") : path->username) ;
  BlueCprintf ("  Password: %S\n",
               path->password == OFC_NULL ? TSTR("NULL") : path->password) ;
  BlueCprintf ("  Domain: %S\n",
               path->domain == OFC_NULL ? TSTR("NULL") : path->domain) ;
  BlueCprintf ("  Num Dirs: %d\n", path->num_dirs) ;
  for (i = 0 ; i < path->num_dirs ; i++)
    BlueCprintf ("    %d: %S\n", i, path->dir[i]) ;
      
  BlueCprintf ("  Absolute: %s\n", path->absolute ? "yes" : "no") ;
  BlueCprintf ("  Remote: %s\n", path->remote ? "yes" : "no") ;
}  

static BLUE_HANDLE hWorkgroups ;

OFC_CORE_LIB OFC_VOID InitWorkgroups (OFC_VOID)
{
  hWorkgroups = BlueQcreate() ;
}

OFC_CORE_LIB OFC_VOID DestroyWorkgroups (OFC_VOID)
{
  OFC_LPTSTR pWorkgroup ;

  for (pWorkgroup = BlueQfirst (hWorkgroups) ;
       pWorkgroup != OFC_NULL ;
       pWorkgroup = BlueQfirst (hWorkgroups))
    RemoveWorkgroup (pWorkgroup);

  BlueQdestroy(hWorkgroups);

  hWorkgroups = BLUE_HANDLE_NULL ;
}

static OFC_LPTSTR FindWorkgroup (OFC_LPCTSTR workgroup)
{
  OFC_LPTSTR pWorkgroup ;

  for (pWorkgroup = BlueQfirst (hWorkgroups) ;
       pWorkgroup != OFC_NULL && BlueCtstrcmp (pWorkgroup, workgroup) != 0 ;
       pWorkgroup = BlueQnext (hWorkgroups, pWorkgroup)) ;

  return (pWorkgroup) ;
}

OFC_CORE_LIB OFC_VOID UpdateWorkgroup (OFC_LPCTSTR workgroup)
{
  OFC_LPTSTR pWorkgroup ;

  BlueLock (lockPath) ;
  pWorkgroup = FindWorkgroup (workgroup) ;

  if (pWorkgroup == OFC_NULL)
    {
      pWorkgroup = BlueCtstrndup (workgroup, OFC_MAX_PATH) ;
      BlueQenqueue (hWorkgroups, pWorkgroup) ;
    }
  BlueUnlock (lockPath) ;
}

OFC_CORE_LIB OFC_VOID RemoveWorkgroup (OFC_LPCTSTR workgroup)
{
  OFC_LPTSTR pWorkgroup ;

  BlueLock (lockPath) ;
  pWorkgroup = FindWorkgroup (workgroup) ;

  if (pWorkgroup != OFC_NULL)
    {
      BlueQunlink (hWorkgroups, pWorkgroup) ;
      BlueHeapFree (pWorkgroup) ;
    }
  BlueUnlock (lockPath) ;
}

OFC_CORE_LIB OFC_BOOL LookupWorkgroup (OFC_LPCTSTR workgroup)
{
  OFC_BOOL ret ;
  OFC_LPTSTR pWorkgroup ;

  BlueLock (lockPath) ;
  pWorkgroup = FindWorkgroup (workgroup) ;

  ret = OFC_FALSE ;
  if (pWorkgroup != OFC_NULL)
    ret = OFC_TRUE ;

  BlueUnlock (lockPath) ;
  return (ret) ;
}

