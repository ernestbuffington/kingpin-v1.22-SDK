// piclib.h


void LoadLBM (char *filename, qboolean explicitPath, byte **picture, byte **palette);
void WriteLBMfile (char *filename, byte *data, int width, int height
	, byte *palette);
void LoadPCX (char *filename, qboolean explicitPath, byte **picture, byte **palette, int *width, int *height);
void WritePCXfile (char *filename, byte *data, int width, int height
	, byte *palette);

// loads / saves either lbm or pcx, depending on extension
void Load256Image (char *name, byte **pixels, byte **palette,
				   int *width, int *height);
void Load256ImageNew (char *name, qboolean explicitPath, byte **pixels,
					  byte **palette, int *width, int *height);	// Knightmare 7/12/12- added relative path support
void Save256Image (char *name, byte *pixels, byte *palette,
				   int width, int height);


void LoadTGA (char *filename, byte **pixels, int *width, int *height);
