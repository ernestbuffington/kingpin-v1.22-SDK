// Argh! - pak.h
// Pak loading functions moved to this separate file, placed 
// in common dir as qbsp3 can also benefit from functions (2.00)

// ArghRad source - modified version of qrad3 source by id Software.
// ArghRad code modifications may not be used without permission
// from author Tim Wright. (Argh!)


// Argh! - pak-related stuff
typedef struct
{
	char magic[4];	// "PACK"
	long diroffset;	// pakdir offset
	long dirsize;	// pakdir size (# entries * 64)
} pakheader_t;

typedef struct pakentry
{
	char filename[56];		// entry name
	long offset;			// pakdata offset
	long size;				// pakdata size
} pakentry_t;

#define MAX_PAKS 10


void LoadPakdirs (void);
void FreePakdirs (void);
int LoadPakFile (char name[1024], byte **data);

