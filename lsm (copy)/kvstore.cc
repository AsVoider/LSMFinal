#include "kvstore.h"
#include "utils.h"
#include <algorithm>
#include <fstream>
#include <string>

pair<uint64_t, uint64_t> findMinMax(vector<pair<uint64_t, uint64_t>> pairs)
{
    uint64_t min = pairs[0].first, max = pairs[1].second;
    for(auto it : pairs)
    {
        max = it.second > max ? it.second : max;
        min = it.first < min ? it.first : min;
    }
    return pair<uint64_t, uint64_t> (min, max);
}

KVStore::KVStore(const std::string &dir1): KVStoreAPI(dir1)
{
    directionary = dir1;
    curr = 0;
    if(utils::dirExists(directionary))
    {
        vector<string> retString;
        int hasLevel = utils::scanDir(directionary, retString);
        sort(retString.begin(), retString.end());
        if(hasLevel > 0)
        {
            for(int i = 0; i < hasLevel; i++)
            {
                levels.push_back(Level(i, i == 0 ? 0 : 1));
                string thisLevel = directionary + "/" + retString[i];
                vector<string> tabString;
                int hasTable = utils::scanDir(thisLevel, tabString);

                for(int j = 0; j < hasTable; j++)
                {
                    Cache* cachej = new Cache(thisLevel + "/" + tabString[j]);
                    if(cachej->header.time > curr)
                        curr = cachej->header.time;
                    levels[i].put(cachej);
                }
                levels[i].Sort();
            }
        }
        else
        {
            utils::mkdir((directionary + "/level-0").c_str());
            levels.push_back(Level(0, 0));
        }
    }
    else
    {
        utils::mkdir(directionary.c_str());
        utils::mkdir((directionary + "/level-0").c_str());
        levels.push_back(Level(0, 0));
    }
    Mem = new Skip();
    curr += 1;
}

KVStore::~KVStore()
{
    if(Mem->num > 0)
        memToSST(directionary + "/level-0", curr++, Mem);
    delete Mem;
    resComp();
    levels.clear();
}

void KVStore::memToSST(const string& dir, const uint64_t& time, Skip* sk)
{
	Cache* tmp = new Cache();
	node* hd = sk->toBottom();
    node* ed = sk->toend();
	BF* f = tmp->bf;
    uint64_t Sz = sk->size();
    string file = dir + "/" + to_string(time) + ".sst";

    //2096050
    tmp->header.mink = hd->k;
    tmp->header.maxk = ed->k;
    tmp->header.num = sk->num;
    tmp->header.time = time;
    tmp->path = file;
	char* buff = new char[32];
    ofstream out(file, ios::binary | ios::out);
    if(!out)
    {
        cout << "Failed to open" + file << endl;
    }
	*(uint64_t*)buff = time;
	*(uint64_t*)(buff + 8) = sk->num;
	*(uint64_t*)(buff + 16) = hd->k;
    *(uint64_t*)(buff + 24) = ed->k;
    out.write(buff, 32);

    char* bfbuff = new char[10240];
    int cc = Sz - 10272;
    char* KOFFVALUE = new char[cc];
    char* idx = KOFFVALUE;
    uint32_t off = 10272 + sk->num * 12;

    for(; hd != nullptr; hd = hd->r)
    {
        f->insert(hd->k);
        *(uint64_t*) idx = hd->k;
        idx = idx + 8;
        *(uint32_t*) idx = off;
        idx = idx + 4;

        tmp->index.emplace_back(hd->k, off);

        uint32_t offs = off + hd->v.size();
        if(offs > Sz)
        {
            cout << "1OverFlow" << endl;
            exit(-1);
        }
        memcpy(KOFFVALUE + off - 10272, hd->v.c_str(), hd->v.size());
        off = offs;
    }

    f->toFile(bfbuff);
    
    out.write(bfbuff, 10240);
    out.write(KOFFVALUE, Sz - 10272);
    out.close();
    delete[] buff;
    delete[] bfbuff;
    delete[] KOFFVALUE;

    levels[0].put(tmp);
}
/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
	if(Mem->size() + s.size() + 12 < maxvalue)
    {
        Mem->put(key, s);
        return;
    }

    memToSST(directionary + "/level-0", curr++, Mem);
    delete Mem;
	Mem = new Skip();
    
	resComp();
	Mem->put(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	string val = Mem->get(key);
	if (val != "")
	{
		if (val == "~DELETED~")
			return "";
		return val;
	}
	//todo: find in sstable
    int cachesz = levels.size();
    for(int i = 0; i < cachesz; i++)
    {
        val = levels[i].getInThisLevel(key);
        if(val != "")
        {
            if(val == "~DELETED~")
                return "";
            else
                return val;
        }
    }
	return "";
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	string val = get(key);
	if (val == "") return 0;
	put(key, "~DELETED~");
	return 1;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
	delete Mem;
	Mem = new Skip();
    levels.clear();
    if(!utils::dirExists(directionary))
    {
        return;
    }
    else
    {
        vector<string> rmDir;
        int rmLevel = utils::scanDir(directionary, rmDir);
        sort(rmDir.begin(), rmDir.end());
        for(int i = 0; i < rmLevel; i++)
        {
            string levelDir = directionary + "/" + rmDir[i];
            vector<string> rmSST;
            int sstNum = utils::scanDir(levelDir, rmSST);
            for(int j = 0; j < sstNum; j++)
                utils::rmfile((levelDir + "/" + rmSST[j]).c_str());
            utils::rmdir(levelDir.c_str());
        }
    }
    utils::mkdir((directionary + "/level-0").c_str());
    levels.push_back(Level(0, 0));
    curr = 0;
}

void KVStore::resComp()
{
    uint32_t siz = levels.size();
    for(uint32_t i = 0; i < siz; i++)
    {
        if(levels[i].getSize() > levels[i].num)
            comp(i);
        else
            break;
    }
}

void KVStore::comp(uint32_t i)
{
    vector<pair<uint64_t, uint64_t>> timePairs;
    vector<SST> bigTable;
    uint64_t min, max;
    levels[i].getUp(bigTable, timePairs);

    if(timePairs.size() > 0)
    {
        pair<uint64_t, uint64_t> a = findMinMax(timePairs);
        min = a.first;
        max = a.second;
    }

    if(++i < levels.size())
    {
        levels[i].getNext(min, max, bigTable);
    }
    else{
        utils::mkdir((directionary + "/level-" + to_string(i)).c_str());
        levels.push_back(Level(levels.size(), 1));
    }

    for(auto it = bigTable.begin(); it != bigTable.end(); ++it)
        utils::rmfile((*it).path.c_str());
    sort(bigTable.begin(), bigTable.end(), cmpSST);
    merge(bigTable);
    if(i == levels.size() - 1)
    {
        bigTable[0].filter();
    }
    vector<Cache*> caches = bigTable[0].saveToCache(directionary + "/level-" + to_string(i));
    for(auto it = caches.begin(); it != caches.end(); ++it) {
        levels[i].put(*it);
    }
    levels[i].Sort();
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string> > &list)
{	
    //string* a = new string[key2 - key1];
}


void KVStore::merge(vector<SST> &ssts)
{
    if(ssts.size() <= 1)
        return;
    vector<SST> BigSST;
    uint32_t i = 0;

    while(ssts.size() > 1)
    {
        //cout << "here" <<endl;
        for(i = 0; i < ssts.size() - 1; i += 2)
        {
            BigSST.push_back(mergetosst(ssts[i], ssts[i+1]));
        }
        if(ssts.size() > i)
        {
            BigSST.push_back(ssts.back());
        }
        ssts = BigSST;
        BigSST = vector<SST>();
    }//finally merge method
}

SST KVStore::mergetosst(SST& t1, SST& t2)
{
    SST newSST;
    bool t1t2 = t1.timeStamp > t2.timeStamp;
    newSST.timeStamp = t1t2 ? t1.timeStamp : t2.timeStamp;
    while(!t1.kvStore.empty() && !t2.kvStore.empty())
    {
        if(t1.kvStore.front().K > t2.kvStore.front().K)
        {
            newSST.kvStore.push_back(t2.kvStore.front());
            t2.kvStore.pop_front();
        }
        else if(t1.kvStore.front().K < t2.kvStore.front().K)
        {
            newSST.kvStore.push_back(t1.kvStore.front());
            t1.kvStore.pop_front();
        }
        else
        {
            if(t1t2)
            {
                newSST.kvStore.push_back(t1.kvStore.front());
            }
            else
            {
                newSST.kvStore.push_back(t2.kvStore.front());
            }
            t1.kvStore.pop_front();
            t2.kvStore.pop_front();
        }
    }
    while(!t1.kvStore.empty())
    {
        newSST.kvStore.push_back(t1.kvStore.front());
        t1.kvStore.pop_front();
    }
    while(!t2.kvStore.empty())
    {
        newSST.kvStore.push_back(t2.kvStore.front());
        t2.kvStore.pop_front();
    }
    return newSST;
}
//�Ȳ���