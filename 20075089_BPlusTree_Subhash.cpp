#include <bits/stdc++.h>
using namespace std;

#define  sz(s)   (int)s.size()
#define  all(s)  (s).begin(),(s).end()

class Node {
public:
    bool isLeaf;
    vector<int>Keys;
    vector<Node *>pointers;
    Node *next;

    Node() {
        isLeaf = true;
        next = NULL;
    }
};

class BPlusTree {
public:

    Node *root;
    int b;
    //b is the fanout

    BPlusTree() {
        root = new Node();
        b = 3;
    }

    BPlusTree(int b) {
        root = new Node();
        this->b = b;
    }

private:

    pair<Node*,int> insert(Node *cur,int id) {
        if(cur->isLeaf) {

            int idx = lower_bound(all(cur->Keys),id)-cur->Keys.begin();
            cur->Keys.insert(cur->Keys.begin()+idx,id);

            if(sz(cur->Keys) < b) return {NULL,-1};
            //overflow in leaf
            Node *new_node = new Node();

            while(sz(cur->Keys) > (b+1)/2) {
                new_node->Keys.push_back(cur->Keys.back());
                cur->Keys.pop_back();
            }

            reverse(all(new_node->Keys));

            new_node->next = cur->next;
            cur->next = new_node;

            return {new_node,cur->Keys.back()};
        }

        int idx = lower_bound(all(cur->Keys),id)-cur->Keys.begin();

        auto res = insert(cur->pointers[idx],id);

        auto child = res.first;
        auto new_id = res.second;

        if(!child) return {NULL,-1};
        
        idx = lower_bound(all(cur->Keys),new_id)-cur->Keys.begin();
        cur->Keys.insert(cur->Keys.begin()+idx,new_id);
        cur->pointers.insert(cur->pointers.begin()+idx+1,child);

        if(sz(cur->Keys) < b) return {NULL,-1};
        //Overflow in internal node
        Node *new_node = new Node();

        new_node->isLeaf = false;

        while(sz(cur->pointers) > b/2+1) {
            new_node->pointers.push_back(cur->pointers.back());
            cur->pointers.pop_back();
        }

        reverse(all(new_node->pointers));

        while(sz(cur->Keys) > (b+1)/2) {
            new_node->Keys.push_back(cur->Keys.back());
            cur->Keys.pop_back();
        }

        int ret_key = -1;

        if(b&1) {
            ret_key = cur->Keys.back();
            cur->Keys.pop_back();
        }
        else {
            ret_key = new_node->Keys.back();
            new_node->Keys.pop_back();
        }

        reverse(all(new_node->Keys));

        return {new_node,ret_key};
    }

    void delete_id(Node *cur,int id) {

        if(cur->isLeaf) {
            auto it = lower_bound(all(cur->Keys),id);
            cur->Keys.erase(it);
            return;
        }

        int idx = lower_bound(all(cur->Keys),id)-cur->Keys.begin();
        delete_id(cur->pointers[idx],id);
        
        if(sz(cur->pointers[idx]->Keys) >= b/2) {
            //No underflow after deletion, so just return
            return;
        }

        //Underflow

        if(cur->pointers[idx]->isLeaf) {
            //underflow in leaf node
            if(idx and sz(cur->pointers[idx-1]->Keys) > b/2) {
                //check if we can adjust from left sibling
                cur->pointers[idx]->Keys.insert(cur->pointers[idx]->Keys.begin(),cur->pointers[idx-1]->Keys.back());
                cur->pointers[idx-1]->Keys.pop_back();
                cur->Keys[idx-1] = cur->pointers[idx-1]->Keys.back();
            }
            else if(idx+1<sz(cur->pointers) and sz(cur->pointers[idx+1]->Keys) > b/2) {
                //check if we can adjust from right sibling
                cur->pointers[idx]->Keys.push_back(cur->pointers[idx+1]->Keys[0]);
                cur->pointers[idx+1]->Keys.erase(cur->pointers[idx+1]->Keys.begin());
                cur->Keys[idx] = cur->pointers[idx]->Keys.back();
            }
            else if(idx) {
                //merge with left sibling
                cur->pointers[idx-1]->next = cur->pointers[idx]->next;
                for(auto u:cur->pointers[idx]->Keys) {
                    cur->pointers[idx-1]->Keys.push_back(u);
                }
                cur->Keys.erase(cur->Keys.begin()+idx-1);
                cur->pointers.erase(cur->pointers.begin()+idx);
            }
            else {
                //merge with right sibling
                cur->pointers[idx]->next = cur->pointers[idx+1]->next;
                for(auto u:cur->pointers[idx+1]->Keys) {
                    cur->pointers[idx]->Keys.push_back(u);
                }
                cur->pointers.erase(cur->pointers.begin()+idx+1);
                cur->Keys.erase(cur->Keys.begin()+idx);
            }

            return;
        }

        //Underflow in internal node
        
        if(idx and sz(cur->pointers[idx-1]->Keys) > b/2) {
            //Adjust from left sibling
            cur->pointers[idx]->Keys.insert(cur->pointers[idx]->Keys.begin(),cur->Keys[idx-1]);
            cur->pointers[idx]->pointers.insert(cur->pointers[idx]->pointers.begin(),cur->pointers[idx-1]->pointers.back());
            cur->pointers[idx-1]->pointers.pop_back();
            cur->Keys[idx] = cur->pointers[idx-1]->Keys.back();
            cur->pointers[idx-1]->Keys.pop_back();
        }
        else if(idx+1<sz(cur->pointers) and sz(cur->pointers[idx+1]->Keys) > b/2) {
            //adjust from right sibling
            cur->pointers[idx]->pointers.push_back(cur->pointers[idx+1]->pointers[0]);
            cur->pointers[idx+1]->pointers.erase(cur->pointers[idx+1]->pointers.begin());
            cur->pointers[idx]->Keys.push_back(cur->Keys[idx]);
            cur->Keys[idx] = cur->pointers[idx+1]->Keys[0];
            cur->pointers[idx+1]->Keys.erase(cur->pointers[idx+1]->Keys.begin());
        }
        else if(idx) {
            //can not adjust, now merge with left sibling
            cur->pointers[idx-1]->Keys.push_back(cur->Keys[idx-1]);

            for(auto u:cur->pointers[idx]->Keys) {
                cur->pointers[idx-1]->Keys.push_back(u);
            }

            for(auto u:cur->pointers[idx]->pointers) {
                cur->pointers[idx-1]->pointers.push_back(u);
            }

            cur->Keys.erase(cur->Keys.begin()+idx-1);
            cur->pointers.erase(cur->pointers.begin()+idx);
        }
        else {
            //merge with right sibling
            cur->pointers[idx+1]->Keys.insert(cur->pointers[idx+1]->Keys.begin(),cur->Keys[idx]);

            while(!cur->pointers[idx]->Keys.empty()) {
                cur->pointers[idx+1]->Keys.insert(cur->pointers[idx+1]->Keys.begin(),cur->pointers[idx]->Keys.back());
                cur->pointers[idx]->Keys.pop_back();
            }

            while(!cur->pointers[idx]->pointers.empty()) {
                cur->pointers[idx+1]->pointers.insert(cur->pointers[idx+1]->pointers.begin(),cur->pointers[idx]->pointers.back());
                cur->pointers[idx]->pointers.pop_back();
            }

            cur->pointers.erase(cur->pointers.begin()+idx);
            cur->Keys.erase(cur->Keys.begin()+idx);
        }
    }

public:
   
   void add_key(int id) {
        //public function to add an id into B+ Tree
        auto res = insert(root,id);

        auto child = res.first;
        auto new_id = res.second;
        
        if(!child) {
            return;
            //child is NULL , so no need to do anything , just return
        }

        //child is not NULL , means Level up is there in our B+ Tree 
        Node *new_node = new Node();
        //create new node and point it to root and new_child pointer created
        new_node->isLeaf = false;
        new_node->Keys.push_back(new_id);
        new_node->pointers.push_back(root);
        new_node->pointers.push_back(child);
        root = new_node;
        //At last assign root to new_node created
   }

   void remove_key(int id) {

        delete_id(root,id);

        if(root->isLeaf) return;
        
        if(root->Keys.empty()) {
            root = root->pointers[0];
            //One level is decreased in our B+ Tree
        }
   }
   
   bool searchKey(Node *cur,int id) {

        if(cur->isLeaf) {
            return binary_search(all(cur->Keys),id);
        }

        int idx = lower_bound(all(cur->Keys),id)-cur->Keys.begin();
        return searchKey(cur->pointers[idx],id);
   }

   void printTreeLeaves(Node *cur) {

        if(cur->isLeaf) {
            for(auto key:cur->Keys) {
                cout<<key<<' ';
            }
            return;
        }

        for(auto ptr:cur->pointers) {
            printTreeLeaves(ptr);
        }
   }

   vector<int> range_search_query(Node* cur, int l,int r) {

        if(cur->isLeaf) {
            vector<int>ans;
            while(cur) {
                for(auto u:cur->Keys) {
                    if(u > r) return ans;
                    if(u >= l) {
                        ans.push_back(u);
                    }
                }
                cur = cur->next;
            }
            
            return ans;
        }

        int idx = lower_bound(all(cur->Keys),l)-cur->Keys.begin();
        return range_search_query(cur->pointers[idx],l,r);
   }
};

int main() {
    
    int b;
    cout<<"Please enter fanout value :- \n";
    cin>>b;

    BPlusTree tr(b);

    while(true) {
        cout<<"Press 1 to insert an id.\n";
        cout<<"Press 2 to delete an id.\n";
        cout<<"Press 3 to search an id.\n";
        cout<<"Press 4 for range search query.\n";
        cout<<"Press any other key to quit.\n";
        int option;
        cin>>option;
        if(option == 1) {
            int id;
            cout<<"Enter id ->\n";
            cin>>id;
            if(tr.searchKey(tr.root,id)) cout<<"Enter unique id.\n";
            else {
                tr.add_key(id);
                cout<<"id inserted successfully.\n";
            }
        }
        else if(option == 2) {
            int id;
            cout<<"Enter id ->\n";
            cin>>id;
            if(!tr.searchKey(tr.root,id)) cout<<"Given id does not exist.\n";
            else {
                tr.remove_key(id);
                cout<<"id deleted successfully.\n";
            }
        }
        else if(option == 3) {
            int id;
            cout<<"Enter id ->\n";
            cin>>id;
            if(tr.searchKey(tr.root,id)) cout<<"Given id exists in tree.\n";
            else cout<<"Given id does not exist.\n";
        }
        else if(option == 4) {
            int l,r;
            cout<<"Enter a range ->\n";
            cin>>l>>r;
            vector<int>res = tr.range_search_query(tr.root,l,r);
            cout<<"Following id's exist in given range ->\n";
            for(auto u:res) cout<<u<<' ';cout<<"\n";
        }
        else {
            cout<<"Bye!!!\n\n";
            break;
        }
        cout<<"\n";
    }

    return 0;
}