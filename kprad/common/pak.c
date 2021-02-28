// Argh! - pak.c
// Pak loading functions moved to this separate file, placed 
// in common dir as qbsp3 can also benefit from functions (2.00)

// ArghRad source - modified version of qrad3 source by id Software.
// ArghRad code modifications may not be used without permission
// from author Tim Wright. (Argh!)


#include "cmdlib.h"
#include "pak.h"


// (rev1.50)
byte	*pakdir[MAX_PAKS];	// gamedir pakdirs
byte	*mpakdir[MAX_PAKS];	// moddir  pakdirs
// (1.50)
long	pakentries[MAX_PAKS];	//gamedir # pak entries
long	mpakentries[MAX_PAKS];	//moddir  # pak entries


// Argh! - load all pakdirs into memory (rev1.50)
void LoadPakdirs (void)
{
	int i;
	char pakpath[1024];
	char mpakpath[1024];
	FILE *pakfile;
	pakheader_t header;

	for (i = 0; i < MAX_PAKS; i++)
	{
		pakdir[i]  = NULL;
		mpakdir[i] = NULL;

		pakentries[i]  = 0;
		mpakentries[i] = 0;

		sprintf(pakpath,  "%spak%d.pak", gamedir, i);
		sprintf(mpakpath, "%spak%d.pak", moddir,  i);

		// GAMEdir paks
		if (FileExists(pakpath))
		{
			pakfile = SafeOpenRead(pakpath);
			fread(&header, sizeof(pakheader_t), 1, pakfile);
			pakentries[i] = header.dirsize / sizeof(pakentry_t);	// Knightmare 7/10/12- changed, was 64 bytes

			pakdir[i] = malloc(header.dirsize);
			fseek(pakfile, header.diroffset, SEEK_SET);
			fread(pakdir[i], sizeof(byte), header.dirsize, pakfile);

			fclose(pakfile);
		}

		// MODdir paks
		if (FileExists(mpakpath))
		{
			pakfile = SafeOpenRead(mpakpath);
			fread(&header, sizeof(pakheader_t), 1, pakfile);
			mpakentries[i] = header.dirsize / sizeof(pakentry_t);	// Knightmare 7/10/12- changed, was 64 bytes

			mpakdir[i] = malloc(header.dirsize);
			fseek(pakfile, header.diroffset, SEEK_SET);
			fread(mpakdir[i], sizeof(byte), header.dirsize, pakfile);

			fclose(pakfile);
		}
	}
}

// Argh! - free up memory used by pakdirs (rev1.50)
void FreePakdirs (void)
{
	int i;

	for (i = 0; i < MAX_PAKS; i++)
	{
		free(pakdir[i]);
		free(mpakdir[i]);
	}
}

// Argh! - try to load data from moddir, moddir paks, gamedir,
//         and gamedir paks;  return data size (rev1.50)
int LoadPakFile(char name[1024], byte **data)
{
	int i,j;
	char filename[1024];
	FILE *file;
	long size;
	pakentry_t entry;

	*data = NULL;

	// MODdir
	sprintf(filename, "%s%s", moddir, name);
	if (FileExists(filename))
	{
		file = SafeOpenRead(filename);

		size = Q_filelength(file);
		if (!(*data = malloc(size)))
			Error ("LoadPakFile: out of memory");
		fread(*data, size, 1, file);

		fclose(file);

		return size;
	}

	// MODdir PAKs (check in descending order)
	for (i = MAX_PAKS-1; i > -1; i--)
	{
		if (mpakentries[i] == 0)
			continue;

		for (j = 0; j < mpakentries[i]; j++)
		{
			entry = *(pakentry_t *)(mpakdir[i]+(j*sizeof(pakentry_t)));	// Knightmare 7/10/12- changed, was 64 bytes

			if (!Q_strcasecmp(name, entry.filename))
			{
				sprintf(filename, "%spak%d.pak", moddir, i);
				file = SafeOpenRead(filename);

			//	if (!(*data = malloc(entry.size)))
			//		Error ("LoadPakFile: out of memory");

			//	fseek(file, entry.offset, SEEK_SET);
				if (fseek(file, entry.offset, SEEK_SET) < 0)	// Knightmare changed 7/10/12
				{
					printf("LoadPakFile: Unexpected EOF in pak\n");
					return 0;
				}

				if (!(*data = malloc(entry.size)))
					Error ("LoadPakFile: out of memory");

			//	fread(*data, entry.size, 1, file);
				if (fread(*data, 1, entry.size, file) != entry.size)	// Knightmare changed 7/10/12
				{
					printf("LoadPakFile: Error reading %s from pak\n", filename);
					fclose(file);
					return 0;
				}

				fclose(file);

				return entry.size;
			}
		}
	}

	// GAMEdir
	sprintf(filename, "%s%s", gamedir, name);
	if (FileExists(filename))
	{
		file = SafeOpenRead(filename);

		size = Q_filelength(file);
		if (!(*data = malloc(size)))
			Error ("LoadPakFile: out of memory");
		fread(*data, size, 1, file);

		fclose(file);

		return size;
	}

	// GAMEdir PAKs (check in descending order)
	for (i = MAX_PAKS-1; i > -1; i--)
	{
		if (pakentries[i] == 0)
			continue;

		for (j = 0; j < pakentries[i]; j++)
		{
			entry = *(pakentry_t *)(pakdir[i]+(j*sizeof(pakentry_t)));	// Knightmare 7/10/12- changed, was 64 bytes

			if (!Q_strcasecmp(name, entry.filename))
			{
				sprintf(filename, "%spak%d.pak", gamedir, i);
				file = SafeOpenRead(filename);

			//	if (!(*data = malloc(entry.size)))
			//		Error ("LoadPakFile: out of memory");

			//	fseek(file, entry.offset, SEEK_SET);
				if (fseek(file, entry.offset, SEEK_SET) < 0)	// Knightmare changed 7/10/12
				{
					printf("LoadPakFile: Unexpected EOF in pak\n");
					return 0;
				}

				if (!(*data = malloc(entry.size)))
					Error ("LoadPakFile: out of memory");

			//	fread(*data, entry.size, 1, file);
				if (fread(*data, 1, entry.size, file) != entry.size)	// Knightmare changed 7/10/12
				{
					printf("LoadPakFile: Error reading %s from pak\n", filename);
					fclose(file);
					return 0;
				}

				fclose(file);

				return entry.size;
			}
		}
	}

	return 0;
}
