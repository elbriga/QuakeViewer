#ifndef READMDL_H
#define READMDL_H

#include "QuakeViewer.h"
#include "3d.h"

obj3d_t *readMdl(char *mdlfilename);

typedef float	vec_t;
typedef vec_t	vec3_t[3];
typedef enum {ST_SYNC=0, ST_RAND } synctype_t;

typedef enum { ALIAS_SKIN_SINGLE=0, ALIAS_SKIN_GROUP } aliasskintype_t;
typedef enum { ALIAS_SINGLE=0, ALIAS_GROUP } aliasframetype_t;

typedef struct {
	char	    ident[4];
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;
} mdl_t;

typedef struct {
	int		onseam;
	int		s;
	int		t;
} stvert_t;

typedef struct dtriangle_s {
	int					facesfront;
	int					vertindex[3];
} dtriangle_t;

typedef struct {
	byte	v[3];
	byte	lightnormalindex;
} trivertx_t;

typedef struct {
	trivertx_t	bboxmin;	// lightnormal isn't used
	trivertx_t	bboxmax;	// lightnormal isn't used
	char		name[16];	// frame name from grabbing
} daliasframe_t;

#endif
