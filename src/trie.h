
#include <string.h>
#include <string>

namespace base::trie {

struct Node {
    Node* child[26] {nullptr};
    bool end {false};
};

class Trie {
private:
    Node root;
public:
    /** Initialize your data structure here. */
    Trie() {
        memset(root.child, 0, sizeof(Node*)*26);
    }
    
    void insert(std::string word)  {  
        Node* node = &root;  
        for (auto ch : word) {
            // 关键点：直接访问child数组
            if (node->child[ch - 'a'] == nullptr) {  
                node->child[ch - 'a'] = new Node;  
            }  else {  
            }  
            node = node->child[ch - 'a'];  
        }  
        //到达记录统计单词次数的节点  
        node->end = true;
    } 
    
    /** Returns if the word is in the trie. */
    bool search(std::string word) {
        Node* node = &root; 
        for (auto ch : word) {
            if (node == nullptr) {
                break;
            }
            node = node->child[ch - 'a'];
        }
        bool ans = (node != nullptr && node->end);
        return ans;
    }

    bool search(const char* word) {
        Node* node = &root; 
        for (char ch = word[0]; ch != '\0'; ch++) {
            if (node == nullptr) {
                break;
            }
            node = node->child[ch - 'a'];
        }
        bool ans = (node != nullptr && node->end);
        return ans;
    }    
    
    /** Returns if there is any word in the trie that starts with the given prefix. */
    bool startsWith(std::string prefix) {
        Node* node = &root; 
        int i = 0;
        for (auto ch : prefix) {
            if (node == nullptr) {
                break;
            }
            node = node->child[ch - 'a'];
        }
        bool ans = (node != nullptr);
        return ans;
    }
};

} //namespace