// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

	bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
	numBytes = fileSize;
	numSectors  = divRoundUp(fileSize, SectorSize);

	//EDIT
	int curSector = 0;
	int adSector = 0;
	bool result = true;
	int hdrL1Buf[L1_INDEX_NUM];
	int hdrL2Buf[L1_INDEX_NUM];
	int sectorsGet[NumDirect + L1_INDEX_NUM + L1_INDEX_NUM * L1_INDEX_NUM];
	int getNum = 0;
	L1Index = -1;
	L2Index = -1;

	CreateTime = time(NULL);
	ModifiedTime = time(NULL);
	//预查找空闲地址。将空闲块存入到SectorsGet这个数组中
	IntStatus oldLevel = interrupt -> SetLevel(IntOff);

	if (numSectors > NumDirect)
	{
		if (numSectors > NumDirect + L1_INDEX_NUM)
		{
			if(numSectors > NumDirect + L1_INDEX_NUM + L1_INDEX_NUM * L1_INDEX_NUM) //Max Length
				result = false;
			else 
			{
				adSector = divRoundUp(numSectors - NumDirect - L1_INDEX_NUM, L1_INDEX_NUM);	

				if (freeMap -> NumClear() < 2 + numSectors + adSector)
					result = false;
				else 
				{
					if (freeMap -> NumClear() < numSectors + 2 + L1_INDEX_NUM)
						result = false;
					else //L2
					{
						for (int i = 0; i < 2 + numSectors + adSector; ++ i)
						{
							sectorsGet[i] = freeMap -> Find();
						}

					}

				}

			}
		}
		else 
		{
			if (freeMap -> NumClear() < numSectors + 1)
				result = false;
			else  //L1
			{
				for (int i = 0; i < 1 + numSectors ; ++ i)
				{
					sectorsGet[i]	 = freeMap -> Find();
				}
			}
		}
	}
	else 
	{
		if (freeMap -> NumClear() < numSectors)
		{
			result = false;
		}
		else //Direct
		{
			for (int i = 0; i < numSectors; ++ i)
			{
				sectorsGet[i] = freeMap -> Find();
			}
		}
	}
	(void) interrupt -> SetLevel(oldLevel);
	if (result == false)
		return false;
	

	//在已经分配的空间中的数据清空，并且写回磁盘

	for (int i = 0; i < NumDirect && curSector < numSectors; ++ i, ++ curSector) //Direct
	{ 
		dataSectors[i] = sectorsGet[getNum ++];
		ClearSector(dataSectors[i]);
	}
	if (curSector < numSectors)
	{
		L1Index = sectorsGet[getNum++]; //L1
		ClearSector(L1Index);
		for (int i = 0; i < L1_INDEX_NUM; ++ i)
			hdrL1Buf[i] = -1;
		for (int i = 0; i < L1_INDEX_NUM && curSector < numSectors; ++ i, ++ curSector)
		{
			hdrL1Buf[i] = sectorsGet[getNum ++];
			ClearSector(hdrL1Buf[i]);
		}
		synchDisk -> WriteSector(L1Index, (char *)hdrL1Buf);

		if (curSector < numSectors)
		{
			L2Index = sectorsGet[getNum ++];
			ClearSector(L2Index);

			for (int i = 0; i < L1_INDEX_NUM; ++ i)
			{
				hdrL2Buf[i] = -1;
			}

			for (int i = 0; i < adSector; ++ i)
			{
				hdrL2Buf[i] = sectorsGet[getNum ++];
				ClearSector(hdrL2Buf[i]);
				for (int j = 0; j < L1_INDEX_NUM; ++ j)
					hdrL1Buf[j] = -1;
				for (int j = 0; j < L1_INDEX_NUM && curSector < numSectors; ++ j, ++curSector)
				{
					hdrL1Buf[j] = sectorsGet[getNum ++];
					ClearSector(hdrL1Buf[j]);
				}
				synchDisk -> WriteSector(hdrL2Buf[i], (char *)hdrL1Buf);
			}
			synchDisk -> WriteSector(L2Index, (char *)hdrL2Buf);
		}
	}
	//END


//	if (freeMap->NumClear() < numSectors)
//		return FALSE;		// not enough space
//
//	for (int i = 0; i < numSectors; i++)
//		dataSectors[i] = freeMap->Find();

	return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

	void 
FileHeader::Deallocate(BitMap *freeMap)
{
	//EDIT BY LIHAO
	int curSector = 0;
	int hdrL1Buf[L1_INDEX_NUM];
	int hdrL2Buf[L1_INDEX_NUM];
	IntStatus oldLevel = interrupt -> SetLevel(IntOff);

	for (int i = 0; i < NumDirect && curSector < numSectors; ++ i, ++ curSector) {
		ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
		freeMap->Clear((int) dataSectors[i]);
	}
	if (curSector < numSectors)
	{
		ASSERT(freeMap -> Test(L1Index));
		synchDisk -> ReadSector(L1Index, (char *)hdrL1Buf);

		for (int i = 0; i< L1_INDEX_NUM && curSector < numSectors; ++ i, ++ curSector)
		{
			ASSERT(freeMap -> Test(hdrL1Buf[i]));
			freeMap -> Clear(hdrL1Buf[i]);
		}
		if (curSector < numSectors)
		{
			ASSERT(freeMap -> Test(L2Index));
			synchDisk -> ReadSector(L2Index, (char *)hdrL2Buf);
			for (int j = 0; curSector < numSectors; ++ j)
			{
				ASSERT(freeMap -> Test(hdrL2Buf[j]));
				synchDisk -> ReadSector(hdrL2Buf[j], (char *)hdrL1Buf);
				for (int i = 0; i < L1_INDEX_NUM && curSector < numSectors; ++ i, curSector ++)
				{
					ASSERT(freeMap -> Test(hdrL1Buf[i]));
					freeMap -> Clear(hdrL1Buf[i]);
				}
				freeMap -> Clear(hdrL2Buf[j]);
			}
			freeMap -> Clear(L2Index);
		}
		freeMap -> Clear(L1Index);
	}
	(void )interrupt -> SetLevel(oldLevel);
	//END
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

	void
FileHeader::FetchFrom(int sector)
{
	synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

	void
FileHeader::WriteBack(int sector)
{
	synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

	int
FileHeader::ByteToSector(int offset)
{
	//EDIT BY LIHAO
	int curSector = offset/ SectorSize;
	int hdrL1Buf[L1_INDEX_NUM];
	int hdrL2Buf[L1_INDEX_NUM];
	int adSector = 0;
	int offSector = 0;

	if (curSector < 0)
		return -1;
	if (curSector < NumDirect) //Direct
		return dataSectors[curSector];
	else if (curSector < NumDirect + L1_INDEX_NUM) // L1
	{
		synchDisk -> ReadSector(L1Index, (char *)hdrL1Buf);
		return hdrL1Buf[curSector - NumDirect];
	}
	else if (curSector < NumDirect + L1_INDEX_NUM + L1_INDEX_NUM * L1_INDEX_NUM)
	{
		adSector = (curSector - NumDirect - L1_INDEX_NUM) / L1_INDEX_NUM;
		offSector = (curSector - NumDirect - L1_INDEX_NUM) - adSector * L1_INDEX_NUM;
		synchDisk -> ReadSector(L2Index, (char *)hdrL2Buf);
		synchDisk -> ReadSector(hdrL2Buf[adSector], (char *)hdrL1Buf);
		return hdrL1Buf[offSector];
	}


	//END
	return -1;

}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

	int
FileHeader::FileLength()
{
	return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

	void
FileHeader::Print()
{
	int i, j, k;
	char *data = new char[SectorSize];
	
	//EDIT BY LIHAO
	struct tm *tblock;
	time_t timer = CreateTime;
	tblock  = localtime(&timer);
	printf("\nCreate Time Is %s", asctime(tblock));
	timer = ModifiedTime;
	tblock = localtime(&timer);
	printf("\nModified Time Is %s", asctime(tblock));
	//END

	printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
	for (i = 0; i < numSectors; i++)
		printf("%d ", dataSectors[i]);
	printf("\nFile contents:\n");
	for (i = k = 0; i < numSectors; i++) {
		synchDisk->ReadSector(dataSectors[i], data);
		for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
			if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
				printf("%c", data[j]);
			else
				printf("\\%x", (unsigned char)data[j]);
		}
		printf("\n"); 
	}









	delete [] data;
}
//EDIT
void
FileHeader::SetModifiedTime()
{
	ModifiedTime = time(NULL);
}
void
FileHeader::ClearSector(int sectorNum)
{
	char zero[SectorSize] = {0};
	synchDisk -> WriteSector(sectorNum, (char *)zero);
}

bool
FileHeader:: ChangeSize(BitMap *freeMap, int newSize)
{
	if (newSize > MaxFileSize)
		return false;
	if (newSize <= numBytes)
		return true;
	

	int sectorsGet[NumDirect + L1_INDEX_NUM + L1_INDEX_NUM * L1_INDEX_NUM];
	int newNumSectors = divRoundUp(newSize, SectorSize);
	int curSector;
	int totalCurSectors = 0;
	int adSectors;
	int getNum = 0;
	int hdrL1Buf[L1_INDEX_NUM];
	int hdrL2Buf[L1_INDEX_NUM];
	bool result = true;



	IntStatus oldLevel = interrupt -> SetLevel(IntOff);
	if (newNumSectors <= numSectors)
	{
		(void )interrupt -> SetLevel(oldLevel);
		numBytes = newSize;
		return true;
	}
	if (numSectors > NumDirect)
	{
		if (numSectors > NumDirect + L1_INDEX_NUM)
		{
			if (numSectors > NumDirect + L1_INDEX_NUM + L1_INDEX_NUM * L1_INDEX_NUM)
				result =  false;
			else 	//L2
			{
				adSectors = divRoundUp(numSectors - NumDirect - L1_INDEX_NUM, L1_INDEX_NUM);
				totalCurSectors = numSectors + adSectors + 2;
			}
		}
		else  					//L1
		{
			totalCurSectors = numSectors + 1;
		}
	
	}
	else						//Direct
	{
		totalCurSectors = numSectors;
	}

	if (newNumSectors > NumDirect)
	{
		if (newNumSectors > NumDirect + L1_INDEX_NUM)
		{
			if (newNumSectors > NumDirect + L1_INDEX_NUM + L1_INDEX_NUM * L1_INDEX_NUM)
				result = false;
			else 
			{
				adSectors = divRoundUp(newNumSectors - NumDirect - L1_INDEX_NUM, L1_INDEX_NUM);	
				if (freeMap-> NumClear() < 2 + newNumSectors + adSectors - totalCurSectors)
				{
					result = false;	
				}
				else 			//L2
				{
					for (int i = 0; i < 2 + newNumSectors + adSectors - totalCurSectors;  ++ i)	
					{
						sectorsGet[i] = freeMap -> Find();
					}
				}
			
			}
		}
		else 
		{
			if (freeMap -> NumClear() < newNumSectors + 1 - totalCurSectors)
			{
				result = false;
			}
			else 				//L1
			{
				if (freeMap -> NumClear() < newNumSectors + 1 - totalCurSectors)
				{
					result = false;
				}
				else 
				{
					for (int i = 0; i < 1 + newNumSectors - totalCurSectors; ++ i)
					{
						sectorsGet[i] = freeMap -> Find();
					}
				}
			
			}
		}
	}
	else 	//Direct
	{
		if (freeMap -> NumClear() < newNumSectors - totalCurSectors)
		{
			result = false;
		}
		else 
		{
			for (int i = 0; i < newNumSectors - totalCurSectors; ++ i)
			{
				sectorsGet[i] = freeMap -> Find();
			
			}
		}
	}
	if (result == false)
	{
		(void)interrupt -> SetLevel(oldLevel);
		return false;
	}
	curSector = numSectors;
	numSectors = newNumSectors;
	numBytes = newSize;

	DEBUG('v', "Cur Exist Sector:%d, NewSectorNum:%d\n", curSector, numSectors);
	(void )interrupt -> SetLevel(oldLevel);

	for (int i = curSector; i < NumDirect && curSector < numSectors; ++ i, ++ curSector)// Direct
	{
		dataSectors[i] = sectorsGet[getNum ++];
		ClearSector(dataSectors[i]);
		DEBUG('v', "Insert New Direct Sector:%d in dataSectors[%d]\n", dataSectors[i], i);
	}
	if (curSector < numSectors)
	{
		if (L1Index == -1)
		{
			L1Index = sectorsGet[getNum ++];
			ClearSector(L1Index);
			for (int i = 0; i < L1_INDEX_NUM; ++ i)
				hdrL1Buf[i] = -1;
			DEBUG('v', "Insert New L1Index:%d\n", L1Index);
		}
		else 
		{
			synchDisk -> ReadSector(L1Index, (char * )hdrL1Buf);
		}

		for (int i = curSector - NumDirect; i < L1_INDEX_NUM && curSector < numSectors; ++ i, ++ curSector)
		{
			ASSERT(hdrL1Buf[i] == -1);
			hdrL1Buf[i] = sectorsGet[getNum++];
			ClearSector(hdrL1Buf[i]);
			DEBUG('v', "Insert New L1 Sector:%d in hdrL1Buf[%d]\n", hdrL1Buf[i], i);
		}
		synchDisk -> WriteSector(L1Index, (char *)hdrL1Buf);
		if (curSector < numSectors) //L2
		{
			if (L2Index == -1)
			{
				L2Index = sectorsGet[getNum++];
				ClearSector(L2Index);
				for (int i = 0; i < L1_INDEX_NUM; ++ i)
					hdrL2Buf[i] = -1;
				DEBUG('v', "Insert New L2Index:%d\n", L2Index);
			}
			else
			{
				synchDisk -> ReadSector(L2Index, (char *)hdrL2Buf);
			}
			for (int j = (curSector - (NumDirect + L1_INDEX_NUM)) / L1_INDEX_NUM ; curSector < numSectors ; ++ j)
			{
				if (hdrL2Buf[j] == -1) 
				{
					hdrL2Buf[j] = sectorsGet[getNum++];
					ClearSector(hdrL2Buf[j]);
					for (int i = 0; i < L1_INDEX_NUM; ++ i)
					{
						hdrL1Buf[i] = -1;
					}
				}
				else 
				{
					synchDisk -> ReadSector(hdrL2Buf[j], (char *)hdrL1Buf);
				}
				for (int i = (curSector - (NumDirect + L1_INDEX_NUM)) - (j * L1_INDEX_NUM); i > L1_INDEX_NUM && curSector < numSectors ; ++ i,++ curSector)
				{
					ASSERT(hdrL1Buf[i] == -1);
					hdrL1Buf[i] = sectorsGet[getNum ++ ];
					ClearSector(hdrL1Buf[i]);
				
				
				
				
				
				}
				synchDisk -> WriteSector(hdrL2Buf[j], (char *)hdrL1Buf);
			}
				synchDisk -> WriteSector(L2Index, (char *)hdrL2Buf);
		
		
		
		
		
		}
	
	
	
	}










}
//END
