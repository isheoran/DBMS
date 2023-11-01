#include <bits/stdc++.h>
using namespace std;

#define  sz(s)   (int)s.size()
#define  all(s)  (s).begin(),(s).end()

class Node {
public:
    bool isLeaf;
    vector<int>Keys;
    vector<Node *>pointers;

    Node() {
        isLeaf = true;
    }
};

class BPlusTree {
public:

    Node *root;
    int b;
    //b is the order of internal node

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
            
            Node *new_node = new Node();

            while(sz(cur->Keys) > (b+1)/2) {
                new_node->Keys.push_back(cur->Keys.back());
                cur->Keys.pop_back();
            }

            reverse(all(new_node->Keys));

            return {new_node,cur->Keys.back()};
        }

        int idx = lower_bound(all(cur->Keys),id)-cur->Keys.begin();

        auto [child,new_id] = insert(cur->pointers[idx],id);

        if(!child) return {NULL,-1};

        idx = lower_bound(all(cur->Keys),new_id)-cur->Keys.begin();
        cur->Keys.insert(cur->Keys.begin()+idx,new_id);
        cur->pointers.insert(cur->pointers.begin()+idx+1,child);

        if(sz(cur->Keys) < b) return {NULL,-1};

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

public:
   
   void add_key(int id) {
        auto [child,new_id] = insert(root,id);
        if(!child) return;
        Node *new_node = new Node();
        new_node->isLeaf = false;
        new_node->Keys.push_back(new_id);
        new_node->pointers.push_back(root);
        new_node->pointers.push_back(child);
        root = new_node;
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
};

int main() {
    
    BPlusTree tr(3);

    tr.add_key(2);
    tr.add_key(4);
    tr.add_key(6);
    tr.add_key(1);
    tr.add_key(0);

    tr.add_key(3);

    tr.printTreeLeaves(tr.root);

    return 0;
}