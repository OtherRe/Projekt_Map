#ifndef AISDI_MAPS_TREEMAP_H
#define AISDI_MAPS_TREEMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <cassert>

namespace aisdi
{

template <typename KeyType, typename ValueType>
class TreeMap
{
    struct Node
    {
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<key_type, mapped_type>;
        using size_type = std::size_t;

        Node(key_type key, mapped_type value) : pair(key, value) {}
        /*Node(const Node& other) : pair(other.pair.first, other.pair.second), height(other.height)
        {
            if(other.leftChild){
                leftChild = new Node();
                *leftChild = *other.leftChild;
                leftChild->parent = this;
            }
            if(other.rightChild){
                rightChild->parent = this;
                *rightChild = *other.rightChild;
                rightChild = other.rightChild;
            }

        }
        
        operator = (const Node& other) : pair(other.pair.first, other.pair.second), height(other.height)
        {
            if(other.leftChild){
                leftChild = new Node();
                *leftChild = *other.leftChild;
                leftChild->parent = this;
            }
            if(other.rightChild){
                rightChild->parent = this;
                *rightChild = *other.rightChild;
                rightChild = other.rightChild;
            }

        }*/
        ~Node()
        {
            delete leftChild;
            delete rightChild;
        }

        void insert(Node *node)
        {
            auto current = this;
            while (current->pair.first != node->pair.first)
            {
                if (node->pair.first < current->pair.first)
                {
                    if (current->hasLeftChild())
                        current = current->leftChild;
                    else
                    {
                        current->makeLeftChildOf(node);
                        break;
                    }
                }

                else if (node->pair.first > current->pair.first)
                {
                    if (current->hasRightChild())
                        current = current->rightChild;
                    else
                    {
                        current->makeRightChildOf(node);
                        break;
                    }
                }
            }
        }

        Node *remove()
        {
            if (hasLeftChild() && hasRightChild())
            {
                auto replacement = succesor();
                replacement->remove();
                std::swap(pair, replacement->pair);

                return replacement;
            }
            else if (hasLeftChild())
            {
                makeParentPointTo(leftChild);
                leftChild->parent = this->parent;

                return this;
            }
            else if (hasRightChild())
            {
                makeParentPointTo(rightChild);
                rightChild->parent = this->parent;

                return this;
            }
            makeParentPointTo(nullptr);
            return this;
        }

        Node *find(key_type key)
        {
            auto current = this;
            while (current->pair.first != key)
            {
                if (key < current->pair.first)
                {
                    if (current->hasLeftChild())
                        current = current->leftChild;
                    else
                        return nullptr;
                }

                else if (key > current->pair.first)
                {
                    if (current->hasRightChild())
                        current = current->rightChild;
                    else
                        return nullptr;
                }
            }

            return current;
        }
        Node *succesor() const
        {
            if (hasRightChild())
                return rightChild->min();
            

            auto current = this;
            while (current->parent != nullptr &&
                   current->parent->rightChild == current)
                current = current->parent;

            return current->parent;
        }

        Node *predecesor() const
        {
            if (hasLeftChild())
                return leftChild->max();
            

            auto current = this;
            while (current->parent != nullptr &&
                   current->parent->leftChild == current)
                current = current->parent;

            return current->parent;
        }

        void updateHeight()
        {
            size_t left = leftChild ? leftChild->height : 0;
            size_t right = rightChild ? rightChild->height : 0;
            height = 1 + std::max(left, right);
        }
        
        Node* min()
        {
            if(hasLeftChild())
                return leftChild->min();
            return this;
        }
        
        Node* max()
        {
            if(hasRightChild())
                return rightChild->min();
            return this;
        }

        bool hasLeftChild() const { return leftChild != nullptr; }
        bool hasRightChild() const { return rightChild != nullptr; }

        value_type pair;
        Node *leftChild = nullptr;
        Node *rightChild = nullptr;
        Node *parent = nullptr;
        size_type height = 0;

      private:
        void makeRightChildOf(Node *node)
        {
            rightChild = node;
            node->parent = this;
        }
        void makeLeftChildOf(Node *node)
        {
            leftChild = node;
            node->parent = this;
        }
        void makeParentPointTo(Node *node)
        {
            if (parent != nullptr)
            {
                if (parent->leftChild == this)
                    parent->leftChild = node;
                else
                    parent->rightChild = node;
            }
        }

    }; ///////////NODEEEEE////////

    class AVLTree
    {
      public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<key_type, mapped_type>;
        using size_type = std::size_t;

        AVLTree() : root(nullptr) {}
        AVLTree(const AVLTree &other)
        {
            if (!other.root)
                return;

            /*root = new Node();
            *(root) = *(other.root);*/
            Node *n = other.minNode();
            while (n != nullptr)
            {
                insert(n->pair);
                n = n->succesor();
            }

        }
        AVLTree(AVLTree &&other) : root(nullptr) { std::swap(root, other.root); }
        AVLTree &operator=(const AVLTree &other)
        {
            if(this == &other)
                return *this;

            delete root;
            root = nullptr;

            Node *n = other.minNode();
            while (n != nullptr)
            {
                insert(n->pair);
                n = n->succesor();
            }
            return *this;
        }
        AVLTree &operator=(AVLTree &&other)
        {
            delete root;
            root = nullptr;
            std::swap(root, other.root);
            return *this;
        }

        Node *insert(const value_type &pair)
        {
            return insert(pair.first, pair.second);
        }

        Node *insert(const key_type &key, const mapped_type &value)
        {
            auto newNode = new Node(key, value);
            if (root != nullptr)
            {
                root->insert(newNode);
                rebalance(newNode);
            }
            else
                root = newNode;

            return newNode;
        }

        void remove(key_type key)
        {
            if (root == nullptr)
                throw std::out_of_range("Removing non existing element from tree");

            auto nodeToRemove = root->find(key);
            if (nodeToRemove == nullptr)
                throw std::out_of_range("Removing non existing element from tree");

            nodeToRemove = nodeToRemove->remove();
            if (root == nodeToRemove) // root with one or less child and is deleted
            {
                if (nodeToRemove->leftChild)
                    root = nodeToRemove->leftChild;
                else if (nodeToRemove->rightChild)
                    root = nodeToRemove->rightChild;
                else
                    root = nullptr;
            }
            rebalance(nodeToRemove->parent);

            nodeToRemove->leftChild = nullptr;
            nodeToRemove->rightChild = nullptr;
            delete nodeToRemove;
        }

        Node *findNode(key_type key) const
        {
            if (root == nullptr)
                return nullptr;

            return root->find(key);
             
        }

        Node *minNode() const
        {
            
            return root ? root->min() : root;
        }

        Node *maxNode() const
        {
            return root ? root->max() : root;
        }

        mapped_type &get(key_type key) const
        {
            if (root == nullptr)
                throw std::out_of_range("No such key in the tree");

            Node *node = root->find(key);
            if (node == nullptr)
                throw std::out_of_range("No such key in the tree");

            return node->pair.second;
        }

        bool keyExists(key_type key) const
        {
            if (root == nullptr)
                return false;
            return root->find(key) ? true : false;
        }

        bool isEmpty() const { return root == nullptr; }

        ~AVLTree() { delete root; }

      private:
        Node *root = nullptr;
      //  void copy

        void rebalance(Node *insertedNode)
        {
            auto node = insertedNode;
            while (node)
            {
                node->updateHeight();

                if (height(node->leftChild) >=
                    2 + height(node->rightChild)) // left subtree is bigger than right
                {
                    if (height(node->leftChild->rightChild) >
                        height(node->leftChild->leftChild)) // right subtree of left child
                        leftRotate(node->leftChild);
                    rightRotate(node);
                }

                else if (height(node->rightChild) >=
                         2 + height(node->leftChild)) // right subtree is bigger than left
                {
                    if (height(node->rightChild->leftChild) >
                        height(node->rightChild->rightChild)) // left subtree of right child
                        rightRotate(node->rightChild);
                    leftRotate(node);
                }
                node = node->parent;
            }
        }

        static size_t height(Node *node) { return node ? node->height : 0; }

        void rightRotate(Node *node)
        {

            auto temp = node->leftChild;
            assert(temp != nullptr);

            node->leftChild = temp->rightChild;
            if (temp->rightChild != nullptr)
                temp->rightChild->parent = node;

            temp->rightChild = node;
            temp->parent = node->parent;
            if (node->parent != nullptr)
            {
                if (node->parent->leftChild == node)
                    node->parent->leftChild = temp;
                else
                    node->parent->rightChild = temp;
            }
            else
                root = temp;

            node->parent = temp;

            node->updateHeight();
            temp->updateHeight();
        }

        void leftRotate(Node *node)
        {
            auto temp = node->rightChild;
            assert(temp != nullptr);

            node->rightChild = temp->leftChild;
            if (temp->leftChild != nullptr)
                temp->leftChild->parent = node;

            temp->leftChild = node;
            temp->parent = node->parent;
            if (node->parent != nullptr)
            {
                if (node->parent->leftChild == node)
                    node->parent->leftChild = temp;
                else
                    node->parent->rightChild = temp;
            }
            else
                root = temp;

            node->parent = temp;

            node->updateHeight();
            temp->updateHeight();
        }
        
        
    };

  public:
    using key_type = KeyType;
    using mapped_type = ValueType;
    using value_type = std::pair<key_type, mapped_type>;
    using size_type = std::size_t;
    using reference = value_type &;
    using const_reference = const value_type &;

    using tree_type = AVLTree;
    using tree_node = Node *;

    class ConstIterator;
    class Iterator;
    using iterator = Iterator;
    using const_iterator = ConstIterator;

    TreeMap() = default;
    TreeMap(std::initializer_list<value_type> list) : size(list.size())
    {
        for (auto &item : list)
            tree.insert(item);  
    }

    bool isEmpty() const { return size == 0; }

    mapped_type &operator[](const key_type &key)
    {
        if (auto node = tree.findNode(key))
            return node->pair.second;

        ++size;
        return tree.insert(key, ValueType{})->pair.second;
    }

    const mapped_type &valueOf(const key_type &key) const
    {
        return tree.get(key);
    }

    mapped_type &valueOf(const key_type &key)
    {
        return tree.get(key);
    }

    const_iterator find(const key_type &key) const
    {
        return iteratorOfKey(key);
    }

    iterator find(const key_type &key)
    {
        return iterator(iteratorOfKey(key));
    }

    void remove(const key_type &key)
    { //throws std::out_of_range
        tree.remove(key);
        --size;
    }

    void remove(const const_iterator &it)
    {
        auto node = it.elem;
        if (node == nullptr)
            throw std::out_of_range("Removing end iterator");

        tree.remove(node->pair.first);
        --size;
    }

    size_type getSize() const { return size; }

    bool operator==(const TreeMap &other) const // expensive
    {
        auto iter = begin();
        auto otherIter = other.begin();
        while (iter != end() && otherIter != other.end())
        {
            if (*iter != *otherIter)
                return false;
            ++iter;
            ++otherIter;
        }
        return size == other.getSize();
    }

    bool operator!=(const TreeMap &other) const { return !(*this == other); }

    iterator begin() { return iterator(tree.minNode(), tree); }

    iterator end() { return iterator(nullptr, tree); }

    const_iterator cbegin() const
    {
        return const_iterator(tree.minNode(), tree);
    }

    const_iterator cend() const { return const_iterator(nullptr, tree); }

    const_iterator begin() const { return cbegin(); }

    const_iterator end() const { return cend(); }

  private:
    tree_type tree = tree_type();
    size_type size = 0;

    const_iterator iteratorOfKey(const key_type &key) const
    {
        if (auto node = tree.findNode(key))
            return const_iterator(node, tree);

        return cend();
    }
};

template <typename KeyType, typename ValueType>
class TreeMap<KeyType, ValueType>::ConstIterator
{
  public:
    using reference = typename TreeMap::const_reference;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename TreeMap::value_type;
    using pointer = const typename TreeMap::value_type *;
    using tree_type = typename TreeMap::tree_type;
    using tree_node = typename TreeMap<KeyType, ValueType>::tree_node;

    explicit ConstIterator(tree_node elem, const tree_type &tree)
        : elem(elem), tree(tree) {}

    ConstIterator(const ConstIterator &other)
        : elem(other.elem), tree(other.tree) {}

    ConstIterator &operator++()
    {
        if (elem == nullptr)
            throw std::out_of_range("Incrementing end iterator");
        elem = elem->succesor();
        return *this;
    }

    ConstIterator operator++(int)
    {
        auto temp = *this;
        operator++();
        return temp;
    }

    ConstIterator &operator--()
    {
        if (elem == nullptr) // end
        {
            elem = tree.maxNode();
            if (elem == nullptr) // end == begin
                throw std::out_of_range("Decrementing empty iterator");

            return *this;
        }
        elem = elem->predecesor();
        if (elem == nullptr)
            throw std::out_of_range("Decrementing begin iterator");

        return *this;
    }

    ConstIterator operator--(int)
    {
        auto temp = *this;
        operator--();
        return temp;
    }

    reference operator*() const
    {
        if (elem == nullptr)
            throw std::out_of_range("Dereferencing end pointer");

        return elem->pair;
    }

    pointer operator->() const { return &this->operator*(); }

    bool operator==(const ConstIterator &other) const
    {
        return elem == other.elem;
    }

    bool operator!=(const ConstIterator &other) const
    {
        return !(*this == other);
    }

  public:
    tree_node elem;
    const tree_type &tree;
};

template <typename KeyType, typename ValueType>
class TreeMap<KeyType, ValueType>::Iterator
    : public TreeMap<KeyType, ValueType>::ConstIterator
{
  public:
    using reference = typename TreeMap::reference;
    using pointer = typename TreeMap::value_type *;
    using tree_node = typename TreeMap<KeyType, ValueType>::Node *;

    explicit Iterator(tree_node elem, tree_type &tree)
        : ConstIterator(elem, tree) {}

    Iterator(const ConstIterator &other) : ConstIterator(other) {}

    Iterator &operator++()
    {
        ConstIterator::operator++();
        return *this;
    }

    Iterator operator++(int)
    {
        auto result = *this;
        ConstIterator::operator++();
        return result;
    }

    Iterator &operator--()
    {
        ConstIterator::operator--();
        return *this;
    }

    Iterator operator--(int)
    {
        auto result = *this;
        ConstIterator::operator--();
        return result;
    }

    pointer operator->() const { return &this->operator*(); }

    reference operator*() const
    {
        // ugly cast, yet reduces code duplication.
        return const_cast<reference>(ConstIterator::operator*());
    }
};
} // namespace aisdi

#endif /* AISDI_MAPS_MAP_H */
