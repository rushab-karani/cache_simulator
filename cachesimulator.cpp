/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>


using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss


struct config {
	unsigned int L1blocksize;
	unsigned int L1setsize;
	unsigned int L1size;
	unsigned int L2blocksize;
	unsigned int L2setsize;
	unsigned int L2size;
};

/* you can define the cache class here, or design your own data structure for L1 and L2 cache
*/
class cache
{
	config config_params;
	unsigned long IndexBit_L1, tag_L1, Offset_L1, IndexBit_L2, tag_L2, Offset_L2;
	unsigned long L1Row, L1Col, L1Way, L2Row;
	vector<vector<unsigned long> > cache_L1;
	vector<vector<unsigned long> > cache_L2;
	//unsigned int validbit = 1;
	//unsigned int LRUbit = 1;
	//unsigned int TagCol = 1;
	//unsigned int val;
	vector<vector<unsigned long> > validBit_L1, validBit_L2;
	vector<vector<unsigned long> > LRUBit_L1, LRUBit_L2;

public:
	cache(config config_param)
	{
		config_params = config_param;

		Offset_L1 = (unsigned long)log2(config_params.L1blocksize);
		Offset_L2 = (unsigned long)log2(config_params.L2blocksize);

		IndexBit_L1 = (unsigned long)log2((config_params.L1size)*(pow(2, 10)) / (config_params.L1blocksize*config_params.L1setsize));
		IndexBit_L2 = (unsigned long)log2((config_params.L2size)*(pow(2, 10)) / (config_params.L2blocksize*config_params.L2setsize));

		tag_L1 = 32 - IndexBit_L1 - Offset_L1;
		tag_L2 = 32 - IndexBit_L2 - Offset_L2;

		L1Row = (unsigned long)pow(2, IndexBit_L1);
		L2Row = (unsigned long)pow(2, IndexBit_L2);

		//L1Col = tag_L1 + validbit + LRUbit;

		validBit_L1.resize(config_params.L1setsize);
		for (int i = 0; i < config_params.L1setsize; i++)
		{
			validBit_L1[i].resize(L1Row);
		}

		validBit_L2.resize(config_params.L2setsize);
		for (int i = 0; i < config_params.L2setsize; i++)
		{
			validBit_L2[i].resize(L2Row);
		}

		LRUBit_L1.resize(config_params.L1setsize);
		for (int i = 0; i < config_params.L1setsize; i++)
		{
			LRUBit_L1[i].resize(L1Row);
		}

		LRUBit_L2.resize(config_params.L2setsize);
		for (int i = 0; i < config_params.L2setsize; i++)
		{
			LRUBit_L2[i].resize(L2Row);
		}


		cache_L1 = createCache(config_params.L1setsize, L1Row, config_params.L1blocksize);
		cache_L2 = createCache(config_params.L2setsize, L2Row, config_params.L2blocksize);

	}

	vector<vector<unsigned long> > createCache(unsigned int &way, unsigned long &indexRow, unsigned int &blockSize)
	{
		vector<vector<unsigned long> > cache_buff;

		cache_buff.resize((unsigned long)way);
		for (int i = 0; i < way; i++)
		{
			cache_buff[i].resize(indexRow);

		}
		return cache_buff;
	}

	vector<string> getbits(bitset<32> addr)
	{
		string stringAddr = addr.to_string();
		vector<string> bits(6);
		bits[0] = stringAddr.substr(0, tag_L1);
		bits[1] = stringAddr.substr(tag_L1, IndexBit_L1);
		bits[2] = stringAddr.substr(tag_L1 + IndexBit_L1, Offset_L1);
		bits[3] = stringAddr.substr(0, tag_L2);
		bits[4] = stringAddr.substr(tag_L2, IndexBit_L2);
		bits[5] = stringAddr.substr(tag_L2 + IndexBit_L2, Offset_L2);
		return bits;
	}
	vector<vector<unsigned long> > getL1Cache()
	{
		return cache_L1;
	}

	vector<vector<unsigned long> > getL2Cache()
	{
		return cache_L2;
	}

	vector<vector<unsigned long> > getL1validbit()
	{
		return validBit_L1;
	}

	vector<vector<unsigned long> > getL2validbit()
	{
		return validBit_L2;
	}

	vector<vector<unsigned long> > getL1LRUbit()
	{
		return LRUBit_L1;
	}

	vector<vector<unsigned long> > getL2LRUbit()
	{
		return LRUBit_L2;
	}

	void updateL1Cache(vector<vector<unsigned long> > L1cacheupdate)
	{
		cache_L1 = L1cacheupdate;
	}
	void updateL2Cache(vector<vector<unsigned long> > L2cacheupdate)
	{
		cache_L2 = L2cacheupdate;
	}

	void updateL1validbit(vector<vector<unsigned long> > L1validbitupdate)
	{
		validBit_L1 = L1validbitupdate;
	}
	void updateL2validbit(vector<vector<unsigned long> > L2validbitupdate)
	{
		validBit_L2 = L2validbitupdate;
	}

	void updateL1LRUbit(vector<vector<unsigned long> > L1LRUbitupdate)
	{
		LRUBit_L1 = L1LRUbitupdate;
	}
	void updateL2LRUbit(vector<vector<unsigned long> > L2LRUbitupdate)
	{
		LRUBit_L2 = L2LRUbitupdate;
	}


};


int main(int argc, char* argv[]) {


	int cache_event;
	config cacheconfig;
	ifstream cache_params;
	string dummyLine;
	unsigned long way_L1, way_L2;
	cache_params.open(argv[1]);
	while (!cache_params.eof())  // read config file
	{
		cache_params >> dummyLine;
		cache_params >> cacheconfig.L1blocksize;
		cache_params >> cacheconfig.L1setsize;
		cache_params >> cacheconfig.L1size;
		cache_params >> dummyLine;
		cache_params >> cacheconfig.L2blocksize;
		cache_params >> cacheconfig.L2setsize;
		cache_params >> cacheconfig.L2size;
	}

	if (cacheconfig.L1setsize == 0) 
	{
		cacheconfig.L1setsize = cacheconfig.L1size * 1024 / cacheconfig.L1blocksize;
	}
	if (cacheconfig.L2setsize == 0) 
	{
		cacheconfig.L2setsize = cacheconfig.L2size * 1024 / cacheconfig.L2blocksize;
	}

	cache cache_create(cacheconfig);


	// Implement by you: 
	// initialize the hirearch cache system with those configs
	// probably you may define a Cache class for L1 and L2, or any data structure you like




	int L1AcceState = 0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
	int L2AcceState = 0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;


	ifstream traces;
	ofstream tracesout;
	string outname;
	outname = string(argv[2]) + ".out";

	traces.open(argv[2]);
	tracesout.open(outname.c_str());

	string line;
	string accesstype;  // the Read/Write access type from the memory trace;
	string xaddr;       // the address from the memory trace store in hex;
	unsigned int addr;  // the address from the memory trace store in unsigned int;        
	bitset<32> accessaddr; // the address from the memory trace store in the bitset;
	vector<string> bits;
	unsigned long IndexBitval_L1, tagval_L1, Offsetval_L1, IndexBitval_L2, tagval_L2, Offsetval_L2;

	if (traces.is_open() && tracesout.is_open()) {
		while (getline(traces, line)) {   // read mem access file and access Cache

			istringstream iss(line);
			if (!(iss >> accesstype >> xaddr)) { break; }
			stringstream saddr(xaddr);
			saddr >> std::hex >> addr;
			accessaddr = bitset<32>(addr);

			bits = cache_create.getbits(accessaddr);
			string strTagL1val = bits[0];
			string strIndexL1val = bits[1];
			string strOffsetL1val = bits[2];
			string strTagL2val = bits[3];
			string strIndexL2val = bits[4];
			string strOffsetL2val = bits[5];

			char * ptr;

			tagval_L1 = strtol(strTagL1val.c_str(), &ptr, 2);
			tagval_L2 = strtol(strTagL2val.c_str(), &ptr, 2);

			Offsetval_L1 = strtol(strOffsetL1val.c_str(), &ptr, 2);
			Offsetval_L2 = strtol(strOffsetL2val.c_str(), &ptr, 2);

			if (strIndexL1val != "") {
				IndexBitval_L1 = strtol(strIndexL1val.c_str(), &ptr, 2);
			}
			else {
				IndexBitval_L1 = 0;
			}

			if (strIndexL2val != "") {
				IndexBitval_L2 = strtol(strIndexL2val.c_str(), &ptr, 2);
			}
			else {
				IndexBitval_L2 = 0;
			}

			vector<vector<unsigned long> > cache_L1 = cache_create.getL1Cache();
			vector<vector<unsigned long> > cache_L2 = cache_create.getL2Cache();
			vector<vector<unsigned long> > validbit_L1 = cache_create.getL1validbit();
			vector<vector<unsigned long> > validbit_L2 = cache_create.getL2validbit();
			vector<vector<unsigned long> > LRUbit_L1 = cache_create.getL1LRUbit();
			vector<vector<unsigned long> > LRUbit_L2 = cache_create.getL2LRUbit();
			int buff_LRU_L1, buff_LRU_L2;
			//Check Which way is least used.
			for (unsigned long i = 0; i < cacheconfig.L1setsize; i++)
			{
				if (LRUbit_L1[i][IndexBitval_L1] == 0)
				{
					way_L1 = i;
					break;
				}
			}

			for (unsigned long i = 0; i < cacheconfig.L2setsize; i++)
			{
				if (LRUbit_L2[i][IndexBitval_L2] == 0)
				{
					way_L2 = i;
					break;
				}
			}

			// access the L1 and L2 Cache according to the trace;
			if (accesstype.compare("R") == 0)
			{
				//Implement by you:
				// read access to the L1 Cache, 
				//  and then L2 (if required), 
				//  update the L1 and L2 access state variable;

				L1AcceState = RM;
				L2AcceState = RM;
				for (unsigned long i = 0; i < cacheconfig.L1setsize; i++)
				{
					if (tagval_L1 == cache_L1[i][IndexBitval_L1] && validbit_L1[i][IndexBitval_L1] == 1)
					{
						L1AcceState = RH;
						L2AcceState = NA;
						//Update L1
						cache_L1[i][IndexBitval_L1] = tagval_L1;
						validbit_L1[i][IndexBitval_L1] = 1;
						buff_LRU_L1 = LRUbit_L1[i][IndexBitval_L1];
						for (int j = 0; j < cacheconfig.L1setsize; j++)
						{
							if (j == i)
							{
								LRUbit_L1[j][IndexBitval_L1] = cacheconfig.L1setsize - 1;
							}
							else if (LRUbit_L1[j][IndexBitval_L1] > buff_LRU_L1)
							{
								LRUbit_L1[j][IndexBitval_L1] = LRUbit_L1[j][IndexBitval_L1] - 1;
							}
							else if (LRUbit_L1[j][IndexBitval_L1] < buff_LRU_L1)
							{
								LRUbit_L1[j][IndexBitval_L1] = LRUbit_L1[j][IndexBitval_L1];
							}
						}
						//}
						break;
					}
				}
				if (L1AcceState != RH)
				{
					L1AcceState = RM;
					for (unsigned long i = 0; i < cacheconfig.L2setsize; i++)
					{
						if (tagval_L2 == cache_L2[i][IndexBitval_L2] && validbit_L2[i][IndexBitval_L2] == 1)
						{
							L2AcceState = RH;
							//Update L2
							cache_L2[i][IndexBitval_L2] = tagval_L2;
							validbit_L2[i][IndexBitval_L2] = 1;
							buff_LRU_L2 = LRUbit_L2[i][IndexBitval_L2];
							for (int j = 0; j < cacheconfig.L2setsize; j++)
							{
								if (j == i)
								{
									LRUbit_L2[j][IndexBitval_L2] = cacheconfig.L2setsize - 1;
								}
								else if (LRUbit_L2[j][IndexBitval_L2] > buff_LRU_L2)
								{
									LRUbit_L2[j][IndexBitval_L2] = LRUbit_L2[j][IndexBitval_L2] - 1;
								}
								else if (LRUbit_L2[j][IndexBitval_L2] < buff_LRU_L2)
								{
									LRUbit_L2[j][IndexBitval_L2] = LRUbit_L2[j][IndexBitval_L2];
								}
							}
							// update L1
							cache_L1[way_L1][IndexBitval_L1] = tagval_L1;
							validbit_L1[way_L1][IndexBitval_L1] = 1;
							for (int j = 0; j < cacheconfig.L1setsize; j++)
							{
								if (j == way_L1)
								{
									LRUbit_L1[j][IndexBitval_L1] = cacheconfig.L1setsize - 1;
								}
								else if (LRUbit_L1[j][IndexBitval_L1] == 0)
								{
									LRUbit_L1[j][IndexBitval_L1] = 0;
								}
								else
								{
									LRUbit_L1[j][IndexBitval_L1] = LRUbit_L1[j][IndexBitval_L1] - 1;
								}
							}
							break;
						}
					}
					if (L2AcceState != RH)
					{
						L2AcceState = RM;
						//update L1
						validbit_L1[way_L1][IndexBitval_L1] = 1;
						cache_L1[way_L1][IndexBitval_L1] = tagval_L1;
						for (int j = 0; j < cacheconfig.L1setsize; j++)
						{
							if (j == way_L1)
							{
								LRUbit_L1[j][IndexBitval_L1] = cacheconfig.L1setsize - 1;
							}
							else if (LRUbit_L1[j][IndexBitval_L1] == 0)
							{
								LRUbit_L1[j][IndexBitval_L1] = 0;
							}
							else
							{
								LRUbit_L1[j][IndexBitval_L1] = LRUbit_L1[j][IndexBitval_L1] - 1;
							}
						}
						//Update L2
						cache_L2[way_L2][IndexBitval_L2] = tagval_L2;
						validbit_L2[way_L2][IndexBitval_L2] = 1;
						for (int j = 0; j < cacheconfig.L2setsize; j++)
						{
							if (j == way_L2)
							{
								LRUbit_L2[way_L2][IndexBitval_L2] = cacheconfig.L2setsize - 1;
							}
							else if (LRUbit_L2[j][IndexBitval_L2] == 0)
							{
								LRUbit_L2[j][IndexBitval_L2] = 0;
							}
							else
							{
								LRUbit_L2[j][IndexBitval_L2] = LRUbit_L2[j][IndexBitval_L2] - 1;
							}
						}
					}

				}
			}
			else
			{
				//Implement by you:
			   // write access to the L1 Cache, 
			   //and then L2 (if required), 
			   //update the L1 and L2 access state variable;
				// check in L1 cache if the data exits
				L1AcceState = WM;
				L2AcceState = WM;
				for (unsigned long i = 0; i < cacheconfig.L1setsize; i++)
				{
					if (tagval_L1 == cache_L1[i][IndexBitval_L1] && validbit_L1[i][IndexBitval_L1] == 1)
					{
						L1AcceState = WH;
						//L2AcceState = WH;
						//update L1
						validbit_L1[i][IndexBitval_L1] = 1;
						cache_L1[i][IndexBitval_L1] = tagval_L1;
						buff_LRU_L1 = LRUbit_L1[i][IndexBitval_L1];
						for (int j = 0; j < cacheconfig.L1setsize; j++)
						{
							if (j == i)
							{
								LRUbit_L1[j][IndexBitval_L1] = cacheconfig.L1setsize - 1;
							}
							else if (LRUbit_L1[j][IndexBitval_L1] < buff_LRU_L1)
							{
								LRUbit_L1[j][IndexBitval_L1] = LRUbit_L1[j][IndexBitval_L1];
							}
							else if (LRUbit_L1[j][IndexBitval_L1] > buff_LRU_L1)
							{
								LRUbit_L1[j][IndexBitval_L1] = LRUbit_L1[j][IndexBitval_L1] - 1;
							}
						}
						/*//Update L2
						cache_L2[way_L2][IndexBitval_L2] = tagval_L2;
						validbit_L2[way_L2][IndexBitval_L2] = 1;
						buff_LRU_L2 = LRUbit_L2[way_L2][IndexBitval_L2];
						for (int j = 0; j < cacheconfig.L2setsize; j++)
						{
							if (j == way_L1)
							{
								LRUbit_L2[way_L2][IndexBitval_L2] = cacheconfig.L2setsize - 1;
							}
							else if (LRUbit_L2[j][IndexBitval_L2] < buff_LRU_L2)
							{
								LRUbit_L2[j][IndexBitval_L2] = LRUbit_L2[j][IndexBitval_L2];
							}
							else if (LRUbit_L2[j][IndexBitval_L2] > buff_LRU_L2)
							{
								LRUbit_L2[j][IndexBitval_L2] = LRUbit_L2[j][IndexBitval_L2] - 1;
							}
						}*/
						break;
					}
				}
				//L1 Miss
				// check in L2 cache
				//if (L1AcceState != WH)
				{
					for (unsigned long i = 0; i < cacheconfig.L2setsize; i++)
					{
						if (tagval_L2 == cache_L2[i][IndexBitval_L2] && validbit_L2[i][IndexBitval_L2] == 1)
						{
							L2AcceState = WH;

							//Update L2
							cache_L2[i][IndexBitval_L2] = tagval_L2;
							validbit_L2[i][IndexBitval_L2] = 1;
							buff_LRU_L2 = LRUbit_L2[i][IndexBitval_L2];
							for (int j = 0; j < cacheconfig.L2setsize; j++)
							{
								if (j == i)
								{
									LRUbit_L2[j][IndexBitval_L2] = cacheconfig.L2setsize - 1;
								}
								else if (LRUbit_L2[j][IndexBitval_L2] < buff_LRU_L2)
								{
									LRUbit_L2[j][IndexBitval_L2] = LRUbit_L2[j][IndexBitval_L2];
								}
								else if (LRUbit_L2[j][IndexBitval_L2] > buff_LRU_L2)
								{
									LRUbit_L2[j][IndexBitval_L2] = LRUbit_L2[j][IndexBitval_L2] - 1;
								}
							}
							break;
						}
					}
					// L1 and L2 Miss
					// if not update the cache
					if (L2AcceState != WH)
					{
						L2AcceState = WM;
					}
				}
			}

			cache_create.updateL1Cache(cache_L1);
			cache_create.updateL2Cache(cache_L2);
			cache_create.updateL1validbit(validbit_L1);
			cache_create.updateL2validbit(validbit_L2);
			cache_create.updateL1LRUbit(LRUbit_L1);
			cache_create.updateL2LRUbit(LRUbit_L2);

			tracesout << L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;


		}
		traces.close();
		tracesout.close();
	}
	else cout << "Unable to open trace or traceout file ";

	return 0;
}