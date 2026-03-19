/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
    struct Node {
        value_type *data;
        Node *prev;
        Node *next;

        Node(value_type *d) : data(d), prev(nullptr), next(nullptr) {}
        Node() : data(nullptr), prev(nullptr), next(nullptr) {}
        ~Node() { if (data) delete data; }
    };

    Node *head;
    Node *tail;
    size_t node_count;
    Hash hash_fn;
    Equal equal_fn;
    static const size_t INITIAL_CAPACITY = 16;
    static const size_t MAX_LOAD_FACTOR = 2;

    struct HashNode {
        Node *node;
        HashNode *next;

        HashNode(Node *n, HashNode *nx) : node(n), next(nx) {}
    };

    HashNode **hash_table;
    size_t hash_capacity;
    size_t hash_size;

    void init_hash_table() {
        hash_table = new HashNode*[hash_capacity];
        for (size_t i = 0; i < hash_capacity; ++i) {
            hash_table[i] = nullptr;
        }
        hash_size = 0;
    }

    void clear_hash_table() {
        for (size_t i = 0; i < hash_capacity; ++i) {
            HashNode *current = hash_table[i];
            while (current) {
                HashNode *next = current->next;
                delete current;
                current = next;
            }
            hash_table[i] = nullptr;
        }
        hash_size = 0;
    }

    void rehash() {
        if (hash_size < hash_capacity * MAX_LOAD_FACTOR) return;

        size_t old_capacity = hash_capacity;
        hash_capacity *= 2;
        HashNode **old_table = hash_table;

        hash_table = new HashNode*[hash_capacity];
        for (size_t i = 0; i < hash_capacity; ++i) {
            hash_table[i] = nullptr;
        }

        Node *current = head->next;
        while (current != tail) {
            size_t hash_val = hash_fn(current->data->first) % hash_capacity;
            HashNode *new_node = new HashNode(current, hash_table[hash_val]);
            hash_table[hash_val] = new_node;
            current = current->next;
        }

        for (size_t i = 0; i < old_capacity; ++i) {
            HashNode *current = old_table[i];
            while (current) {
                HashNode *next = current->next;
                delete current;
                current = next;
            }
        }
        delete[] old_table;
    }

    Node* find_node(const Key &key) const {
        size_t hash_val = hash_fn(key) % hash_capacity;
        HashNode *current = hash_table[hash_val];

        while (current) {
            if (equal_fn(current->node->data->first, key)) {
                return current->node;
            }
            current = current->next;
        }
        return nullptr;
    }

    void remove_node(Node *node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
        node_count--;
    }

    void remove_from_hash(const Key &key) {
        size_t hash_val = hash_fn(key) % hash_capacity;
        HashNode *current = hash_table[hash_val];
        HashNode *prev = nullptr;

        while (current) {
            if (equal_fn(current->node->data->first, key)) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    hash_table[hash_val] = current->next;
                }
                delete current;
                hash_size--;
                return;
            }
            prev = current;
            current = current->next;
        }
    }

public:
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
	public:
	    Node *node_ptr;
		const linked_hashmap *map_ptr;
		// The following code is written for the C++ type_traits library.
		// Type traits is a C++ feature for describing certain properties of a type.
		// For instance, for an iterator, iterator::value_type is the type that the
		// iterator points to.
		// STL algorithms and containers may use these type_traits (e.g. the following
		// typedef) to work properly.
		// See these websites for more information:
		// https://en.cppreference.com/w/cpp/header/type_traits
		// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
		// About iterator_category: https://en.cppreference.com/w/cpp/iterator
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;


		iterator() : node_ptr(nullptr), map_ptr(nullptr) {}
		iterator(Node *node, const linked_hashmap *map) : node_ptr(node), map_ptr(map) {}
		iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
		    if (node_ptr == nullptr || node_ptr == map_ptr->tail) {
		        throw invalid_iterator();
		    }
		    iterator temp(*this);
		    node_ptr = node_ptr->next;
		    return temp;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
		    if (node_ptr == nullptr || node_ptr == map_ptr->tail) {
		        throw invalid_iterator();
		    }
		    node_ptr = node_ptr->next;
		    return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->head->next) {
		        throw invalid_iterator();
		    }
		    iterator temp(*this);
		    node_ptr = node_ptr->prev;
		    return temp;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->head->next) {
		        throw invalid_iterator();
		    }
		    node_ptr = node_ptr->prev;
		    return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->tail) {
		        throw invalid_iterator();
		    }
		    return *(node_ptr->data);
		}
		bool operator==(const iterator &rhs) const {
		    return node_ptr == rhs.node_ptr;
		}
		bool operator==(const const_iterator &rhs) const {
		    return node_ptr == rhs.node_ptr;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
		    return node_ptr != rhs.node_ptr;
		}
		bool operator!=(const const_iterator &rhs) const {
		    return node_ptr != rhs.node_ptr;
		}

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->tail) {
		        return nullptr;
		    }
		    return node_ptr->data;
		}

		friend class const_iterator;
	};

	class const_iterator {
	public:
	    const Node *node_ptr;
	    const linked_hashmap *map_ptr;
	public:
	    const_iterator() : node_ptr(nullptr), map_ptr(nullptr) {}
	    const_iterator(const Node *node, const linked_hashmap *map) : node_ptr(node), map_ptr(map) {}
		const_iterator(const const_iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}
		const_iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}

		const_iterator operator++(int) {
		    if (node_ptr == nullptr || node_ptr == map_ptr->tail) {
		        throw invalid_iterator();
		    }
		    const_iterator temp(*this);
		    node_ptr = node_ptr->next;
		    return temp;
		}

		const_iterator & operator++() {
		    if (node_ptr == nullptr || node_ptr == map_ptr->tail) {
		        throw invalid_iterator();
		    }
		    node_ptr = node_ptr->next;
		    return *this;
		}

		const_iterator operator--(int) {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->head->next) {
		        throw invalid_iterator();
		    }
		    const_iterator temp(*this);
		    node_ptr = node_ptr->prev;
		    return temp;
		}

		const_iterator & operator--() {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->head->next) {
		        throw invalid_iterator();
		    }
		    node_ptr = node_ptr->prev;
		    return *this;
		}

		const value_type & operator*() const {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->tail) {
		        throw invalid_iterator();
		    }
		    return *(node_ptr->data);
		}

		bool operator==(const const_iterator &rhs) const {
		    return node_ptr == rhs.node_ptr;
		}
		bool operator==(const iterator &rhs) const {
		    return node_ptr == rhs.node_ptr;
		}

		bool operator!=(const const_iterator &rhs) const {
		    return node_ptr != rhs.node_ptr;
		}
		bool operator!=(const iterator &rhs) const {
		    return node_ptr != rhs.node_ptr;
		}

		const value_type* operator->() const noexcept {
		    if (node_ptr == nullptr || node_ptr == map_ptr->head || node_ptr == map_ptr->tail) {
		        return nullptr;
		    }
		    return node_ptr->data;
		}
	};

	/**
	 * TODO two constructors
	 */
	linked_hashmap() : node_count(0), hash_capacity(INITIAL_CAPACITY), hash_size(0) {
	    head = new Node();
	    tail = new Node();
	    head->next = tail;
	    tail->prev = head;
	    init_hash_table();
	}
	linked_hashmap(const linked_hashmap &other) : node_count(0), hash_capacity(other.hash_capacity), hash_size(0) {
	    head = new Node();
	    tail = new Node();
	    head->next = tail;
	    tail->prev = head;
	    init_hash_table();

	    Node *current = other.head->next;
	    while (current != other.tail) {
	        insert(*current->data);
	        current = current->next;
	    }
	}

	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
	    if (this == &other) return *this;

	    clear();
	    delete head;
	    delete tail;
	    clear_hash_table();
	    delete[] hash_table;

	    node_count = 0;
	    hash_capacity = other.hash_capacity;
	    hash_size = 0;
	    head = new Node();
	    tail = new Node();
	    head->next = tail;
	    tail->prev = head;
	    hash_table = new HashNode*[hash_capacity];
	    for (size_t i = 0; i < hash_capacity; ++i) {
		    hash_table[i] = nullptr;
		}

	    Node *current = other.head->next;
	    while (current != other.tail) {
	        insert(*(current->data));
	        current = current->next;
	    }

	    return *this;
	}

	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
	    clear();
	    delete head;
	    delete tail;
	    clear_hash_table();
	    delete[] hash_table;
	}

	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
	    Node *node = find_node(key);
	    if (node == nullptr) {
	        throw index_out_of_bound();
	    }
	    return node->data->second;
	}
	const T & at(const Key &key) const {
	    Node *node = find_node(key);
	    if (node == nullptr) {
	        throw index_out_of_bound();
	    }
	    return node->data->second;
	}

	/**
	 * TODO
	 * access specified element
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
	    Node *node = find_node(key);
	    if (node != nullptr) {
	        return node->data->second;
	    }

	    insert(value_type(key, T()));
	    node = find_node(key);
	    return node->data->second;
	}

	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
	    return at(key);
	}

	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
	    return iterator(head->next, this);
	}
	const_iterator cbegin() const {
	    return const_iterator(head->next, this);
	}

	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
	    return iterator(tail, this);
	}
	const_iterator cend() const {
	    return const_iterator(tail, this);
	}

	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
	    return node_count == 0;
	}

	/**
	 * returns the number of elements.
	 */
	size_t size() const {
	    return node_count;
	}

	/**
	 * clears the contents
	 */
	void clear() {
	    Node *current = head->next;
	    while (current != tail) {
	        Node *next = current->next;
	        delete current;
	        current = next;
	    }

	    // Clear hash table too
	    for (size_t i = 0; i < hash_capacity; ++i) {
	        HashNode *current = hash_table[i];
	        while (current) {
	            HashNode *next = current->next;
	            delete current;
	            current = next;
	        }
	        hash_table[i] = nullptr;
	    }

	    node_count = 0;
	    hash_size = 0;
	    head->next = tail;
	    tail->prev = head;
	}

	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion),
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
	    Node *existing = find_node(value.first);
	    if (existing != nullptr) {
	        return pair<iterator, bool>(iterator(existing, this), false);
	    }

	    rehash();

	    Node *new_node = new Node(new value_type(value));

	    tail->prev->next = new_node;
	    new_node->prev = tail->prev;
	    new_node->next = tail;
	    tail->prev = new_node;

	    size_t hash_val = hash_fn(value.first) % hash_capacity;
	    HashNode *hash_node = new HashNode(new_node, hash_table[hash_val]);
	    hash_table[hash_val] = hash_node;
	    hash_size++;
	    node_count++;

	    return pair<iterator, bool>(iterator(new_node, this), true);
	}

	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
	    if (pos.node_ptr == nullptr || pos.node_ptr == tail || pos.node_ptr == head) {
	        throw invalid_iterator();
	    }

	    remove_from_hash(pos.node_ptr->data->first);
	    remove_node(pos.node_ptr);
	}

	/**
	 * Returns the number of elements with key
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
	    return find_node(key) != nullptr ? 1 : 0;
	}

	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
	    Node *node = find_node(key);
	    if (node == nullptr) {
	        return end();
	    }
	    return iterator(node, this);
	}
	const_iterator find(const Key &key) const {
	    Node *node = find_node(key);
	    if (node == nullptr) {
	        return cend();
	    }
	    return const_iterator(node, this);
	}
};

}

#endif
