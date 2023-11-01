#include <iostream>
#include <vector>
#include <string>
using namespace std;

class Record {
    int id;
    string record;

public:
	//id  = -1 means this record is empty
	Record() {
		id = -1;
	}

	Record(int id,string record) {
		this->id = id;
		this->record = record;
	}

    string getRecord() {
    	return this->record;
    }

    int getId() {
    	return this->id;
    }

    void setRecord(string record) {
    	this->record = record;
    }

    void setId(int id) {
    	this->id = id;
    }
};

//I am taking Bucket size  = 4
class Bucket {
	Record records[4];
	int local_depth;
public:
	Bucket(){
		local_depth = 0;
	}

	string getRecord(int id) {
		for(int i=0;i<4;i++) {
			if(records[i].getId() == id) {
				return records[i].getRecord();
			}
		}

		return "Record not found";
	}

	void setRecord(int id,string updatedRecord) {
		for(int i=0;i<4;i++) {
			if(records[i].getId() == id) {
				records[i].setRecord(updatedRecord);
				return;
			}
		}
	}

	bool insertRecord(int id,string record) {
		for(int i=0;i<4;i++) {
			if(records[i].getId() == -1) {
				records[i].setId(id);
				records[i].setRecord(record);
				return true;
			}
		}
		return false;
	}

	int getLocalDepth() {
		return local_depth;
	}

	void setLocalDepth(int local_depth) {
		this->local_depth = local_depth;
	}

	int getIdStoredAtIdx(int idx) {
		return records[idx].getId();
	}

	void deleteRecord(int id) {
		for(int i=0;i<4;i++) {
			if(records[i].getId() == id) {
				records[i].setId(-1);
				records[i].setRecord("");
				return;
			}
		}
	}
};

class Directory {
	//mp[i] contains the pointer to bucket containing a Record
    vector<Bucket *>mp;
    int global_depth;
public:
	Directory() {
		global_depth = 0;
		mp = {new Bucket()};
	}

    //Hash Function
	int getHash(int id) {
		return id%(int)(1<<(global_depth));
	}

	//Hash of last k bits
	int getHash(int id,int k) {
		return id%(int)(1<<k);
	}

	int getGlobalDepth() {
		return this->global_depth;
	}
    
	//Find the Record String corresponding to id
	string getRecord(int id) {
		return mp[getHash(id)]->getRecord(id);
	}
    
	int insertRecord(int id,string record) {
		int hash = getHash(id);
        
		//Check if id is already present
		for(int i=0;i<4;i++) {
			if(mp[hash]->getIdStoredAtIdx(i) == id) {
				return -1;
			}
		}

		if(mp[hash]->insertRecord(id,record)) {
			return 1;
		}

		//Corresponding Bucket is not Empty

		if(this->global_depth == mp[hash]->getLocalDepth()) {
            //Double the direcory size
			for(int i=0;i<(1<<global_depth);i++) {
				mp.push_back(mp[i]);
			}

			this->global_depth++;

			//Split Bucket pointed by mp[hash]
            mp[hash|(1<<(global_depth-1))] = new Bucket();

			//Redistribute records present in Bucket pointed by mp[hash]
			for(int i=0;i<4;i++) {
				if(mp[hash]->getIdStoredAtIdx(i)&(1<<(global_depth-1))) {
                    int id = mp[hash]->getIdStoredAtIdx(i);
					mp[hash|(1<<(global_depth-1))]->insertRecord(id,mp[hash]->getRecord(id));
					mp[hash]->deleteRecord(id);
				} 
			}

			mp[hash]->setLocalDepth(mp[hash]->getLocalDepth()+1);
			mp[hash|(1<<(global_depth-1))]->setLocalDepth(mp[hash]->getLocalDepth());
		}
		else {
			//Split the Bucket
			Bucket *new_Bucket = new Bucket();
			//Redistribute records present in Bucket pointed by mp[hash]
			for(int i=0;i<4;i++) {
				if(mp[hash]->getIdStoredAtIdx(i)&(1<<mp[hash]->getLocalDepth())) {
                    int id = mp[hash]->getIdStoredAtIdx(i);
					new_Bucket->insertRecord(id,mp[hash]->getRecord(id));
					mp[hash]->deleteRecord(id);
				} 
			}

			new_Bucket->setLocalDepth(mp[hash]->getLocalDepth()+1);
			mp[hash]->setLocalDepth(new_Bucket->getLocalDepth());

			hash |= (1<<(new_Bucket->getLocalDepth()-1));
			for(int i=0;i<(1<<this->global_depth);i++) {
				if(getHash(i) == hash) {
					mp[i] = new_Bucket;
				}
			}
		}

		hash = this->getHash(id);

		if(mp[hash]->insertRecord(id,record)) return 1;
		return 0;
	}

	bool merge(int id) {
		int hash = getHash(id);
		int l = mp[hash]->getLocalDepth();
		//LocalDepth is zero can not be merged further
		if(l == 0) return false;
		int cnt_records = 0;
		for(int i=0;i<4;i++) {
			cnt_records += (mp[hash]->getIdStoredAtIdx(i)!=-1);
		}

		hash ^= 1<<(l-1);
		if(mp[hash]->getLocalDepth() != l) return false;

		for(int i=0;i<4;i++) {
			cnt_records += (mp[hash]->getIdStoredAtIdx(i)!=-1);
		} 

		if(cnt_records > 4) {
			//We can not merge Buckets
			return false;
		}
        
		hash |= 1<<(l-1);
		//We can merge Buckets
		for(int i=0;i<4;i++) {
			id = mp[hash]->getIdStoredAtIdx(i);
			if(id != -1) {
				mp[hash^(1<<(l-1))]->insertRecord(id,mp[hash]->getRecord(id));
			}
		}

		delete(mp[hash]);

		for(int i=0;i<(1<<global_depth);i++) {
			if(hash == getHash(i,l)) {
                mp[i] = mp[hash^(1<<(l-1))];
			}
		}

		mp[hash^(1<<(l-1))]->setLocalDepth(l-1);

		l = -1;
		for(int i=0;i<(1<<global_depth);i++) {
			l = max(l,mp[i]->getLocalDepth());
		}

		if(global_depth == l) {
			//Directory can not be halved
			return true;
		}

		//global_depth > max({local_depths ...}) So, Directory size can be halved
		global_depth--;
		while((int)mp.size() > (1<<global_depth)) {
			mp.pop_back();
		}

		return true;
	}

	void deleteRecord(int id) {
		int hash = this->getHash(id);
        mp[hash]->deleteRecord(id);
		//Run this while loop until their is a possibility of merging
		while(merge(id));
	}

	void updateRecord(int id,string updatedRecord) {
		int hash = getHash(id);
		mp[hash]->setRecord(id,updatedRecord);
	}
};

int main() {

    Directory dr;

    while(true) {
		cout<<"Press 1 to Insert Record\n";
		cout<<"Press 2 to Delete Record\n";
		cout<<"Press 3 to Update Record\n";
		cout<<"Press 4 to Find Record\n";
		cout<<"Press any other key to exit\n";

		int option;
		cin>>option;

		if(option == 1) {
			int id;
			string record;
			cout<<"Please enter an id and corresponding record string\n";
			cin>>id>>record;
            while(true) {
				int res = dr.insertRecord(id,record);
				if(res == 1) {
					cout<<"Successfully entered the record.\n";
					break;
				}
				else if(res  == -1) {
					cout<<"Duplicate id is already present please enter unique id !!!\n";
					break;
				}
			}
		}
		else if(option == 2) {
			int id;
			cout<<"Please enter an id to delete corresponding record\n";
			cin>>id;
			dr.deleteRecord(id);
		}
		else if(option == 3) {
			int id;
			string record;
			cout<<"Please enter an id and corresponding record string to Update\n";
			cin>>id>>record;
            dr.updateRecord(id,record);
			cout<<"Record successfully updated...\n";
		}
		else if(option == 4) {
			int id;
			cout<<"Please enter an id to find corresponding record\n";
			cin>>id;
			cout<<"Folling is the record corresponding to given id -> \n";
			cout<<dr.getRecord(id);
		}
		else {
			cout<<"Bye...\n";
			break;
		}
		cout<<"\n\n";
	}

	return 0;
}
