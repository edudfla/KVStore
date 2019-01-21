#ifndef KVStore_hpp
#define KVStore_hpp
/*
    The objective is to create a simple key value store.
    To improve search speed and maybe storage efficiency the organization will
be as follows:
    Suppose there are the following keys of type int8_t which represent the
following list of words:
    "Quick setup - if you’ve done this kind of thing before"
    Inserting one by one will generate the successive trees:

               Q           
              /|\          
             / u s         
            /  | |\        
           /   i e \       
          /    | |  \      
         /     c t   \     
        /      | u    \    
       /       k |     \   
      /          |      y  
     /           p     /|  
    /                 / o  
   -                 /  |  
    \               t   u  
     \              |   |  
      \             h   '  
       \            |   |  
        \           i   v  
         i          |\  |  
        /|\         s n e  
       d f k          |    
      /|   |\         g    
     b O   i o             
     | |   | |             
     e n   n f             
     | |   |               
     f e   d               
     |                     
     o                     
     |                     
     r                     
     |                     
     e                     
                           

    In the above graph, slashes and backslashes denote traveling in the same
level (all first letters of every word for instance) and pipes mean going from
one level to the next (from the first letter to the second letter for instance).
    For a little group of words there is no gain at all (much probably ther will
be even loss). But with a large enough set of words, the reuse of nodes like
what can be seen with "this" and "thing" will become prevalent.
    The great gain however happens when the tree is balanced and a search is
done because in this case, for instance, if all 256 symbols are used for the
int8_t type then a word with 10 symbols will take up to 80 compares to be found,
worst case scenario, regardless of the number of words in the tree.

    v_* are the internal state properties.
    f_* are the internal methods.
    *_t are the internal types.

    Is it valuable make node_t a simple data structure or a full fledged class?
    Is it valuable differentiate store from update or merge both in one single
method?
    Is it valuable to wrap the multiple pointers to node_t as std::unique_ptr to
guarantee that they will be automatically freed when no more used? What is the
performance penalty?
    Is it possible to restrict the valid types for key_type_name to only signed
and unsigned integer types wit 8, 16, 32 and 64 bit?
    Where is the best place to put the logic: in the container (KVStore) or in
the contents (node_t)?
	Is it needed a container at all? Looks like all logic will be inside the node
class anyway.
*/
#include "pch.h"

#include <cstdint>
#include <memory>
#include <cassert>

template <typename key_type_name, typename value_type_name>
class KVStore {
public:
	static const uint32_t error_null_key = 1;
	static const uint32_t error_empty_key = 2;
	static const uint32_t error_key_not_found = 3;
	static const uint32_t error_not_assigned = 4;
	static const uint32_t error_empty_tree = 5;
    KVStore() : m_root_node(nullptr) {
    }
	~KVStore() {
		delete m_root_node;
	}
    uint32_t store_or_update(const key_type_name* key, const size_t key_length, const value_type_name& value) {
        if (!key) return error_null_key;
        if (!key_length) return error_empty_key;
        if (!m_root_node) {
			m_root_node = new node_t(key, key_length, value);
			return 0;
        }
		m_root_node->store(key, key_length, value);
		return 0;
    }
    uint32_t retrieve(const key_type_name* key, const size_t key_length, value_type_name& value) {
		if (nullptr == m_root_node) {
			return error_empty_tree;
		}
		return m_root_node->retrieve(key, key_length, value);
    }
	uint32_t remove(const key_type_name* key, const size_t key_length) {
		if (!key) return error_null_key;
		if (!key_length) return error_empty_key;
		if (!m_root_node) {
			return error_empty_tree;
		}
		m_root_node->remove(key, key_length);
		return 0;
	}
    uint32_t remove(const key_type_name* key, size_t key_length) {
    }
	void dump_basic_test() {
		if (m_root_node) {
			m_root_node->dump_basic_test();
		}
    }
    uint32_t balance() {
    }
private:
	class node_t;
	typedef node_t* node_p;
	class node_t {
	public:
		void* operator new(size_t size) {
			++m_allocated_nodes;
			return malloc(size);
		}
		void operator delete(void* mem_ptr) {
			if (nullptr == memptr) {
				return;
			}
			assert(m_allocated_nodes);
			--m_allocated_nodes;
			free(mem_ptr);
		}
		node_t(const key_type_name* key, const size_t key_length, const value_type_name& value)
			: m_key_atom(*key)
			, m_assigned(1 == key_length)
			, m_value(1 == key_length ? new value_type_name(value) : nullptr)
			, m_rightward(nullptr)
			, m_leftward(nullptr)
			, m_downward(1 == key_length ? nullptr : new node_t(key + 1, key_length - 1, value))
		{
		}
		~node_t() {
			// Is the destruction order meaningful?
			delete m_value;
			delete m_rightward;
			delete m_leftward;
			delete m_downward;
		}
		uint32_t store(const key_type_name* key, const size_t key_length, const value_type_name& value) {
			size_t matched_length = 0;
			n_ward_t n_ward = exact_match;
			node_p closest_node = find_closest(key, key_length, matched_length, n_ward);
			return closest_node->store_internal(key + matched_length, key_length - matched_length, n_ward, value);
		}
		uint32_t retrieve(const key_type_name* key, const size_t key_length, value_type_name& value) {
			size_t matched_length = 0;
			n_ward_t n_ward = exact_match;
			node_p node = find_closest(key, key_length, matched_length, n_ward);
			if (key_length != matched_length) {
				return error_key_not_found;
			}
			if (false == node->m_assigned) {
				return error_not_assigned;
			}
			value = *node->m_value;
			return 0;
		}
		uint32_t remove(const key_type_name* key, const size_t key_length) {
			//
			// The remove operation is sui generis in the sense that
			// it needes a sort of m_backward (a reverse) pointer.
			// Since this reverse pointer is not useful at anu other occasion,
			// it is not worth keeping a permanent reference on it, so the
			// temporary recursive stack reference is enouth.
			//
			if (!key) {
				return error_null_key;
			}
			if (!key_length) {
				return error_empty_key;
			}
			if (!m_root_node) {
				return error_empty_tree;
			}
			return m_root_node->remove_internal(key, key_length);
		}
		void dump_basic_test() {
			if (m_leftward) {
				m_leftward->dump_basic_test();
			}
			std::cout << m_key_atom;
			if (m_assigned) {
				std::cout << '(' << *m_value << ')';
			}
			if (m_downward) {
				m_downward->dump_basic_test();
			} else {
				std::cout << ';';
			}
			if (m_rightward) {
				m_rightward->dump_basic_test();
			}
		}
	private:
        key_type_name m_key_atom;
        bool m_assigned;
        value_type_name* m_value;
        node_p m_rightward;
        node_p m_leftward;
        node_p m_downward;
		static size_t m_allocated_nodes = 0;
        // Recursive constructor. How to differentiate when names are shared between properties and parameter?
		typedef enum {
			exact_match = 0,
			leftward = 1,
			downward = 2,
			rightward = 3,
		} n_ward_t;
		node_p find_closest(const key_type_name* key, size_t const key_length, size_t& matched_length, n_ward_t& n_ward) {
			if (*key == m_key_atom) {
				++matched_length;
				++key;
				if (key_length == matched_length) {
					// Exact match
					n_ward = exact_match;
					return this;
				}
				if (nullptr == m_downward) {
					// Partial match.
					// The tree only has the trailing portion of the key.
					// matched_length contains the length of the trailing portion that matched.
					n_ward = downward;
					return this;
				}
				// matched_length elements of key were matched.
				// Go on looking forward the next element of key.
				return m_downward->find_closest(key, key_length, matched_length, n_ward);
			}
			if (*key > m_key_atom) {
				if (nullptr == m_rightward) {
					// No more places to go.
					// key_length contains the matched length so far.
					n_ward = rightward;
					return this;
				}
				return m_rightward->find_closest(key, key_length, matched_length, n_ward);
			}
			if (nullptr == m_leftward) {
				// No more places to go.
				n_ward = leftward;
				return this;
			}
			return m_leftward->find_closest(key, key_length, matched_length, n_ward);
		}
		uint32_t store_internal(const key_type_name* key, size_t key_length, n_ward_t n_ward, const value_type_name& value) {
			switch (n_ward) {
			case exact_match:
				*m_value = value;
				m_assigned = true;
				break;
			case leftward:
				m_leftward = new node_t(key, key_length, value);
				break;
			case downward:
				m_downward = new node_t(key, key_length, value);
				break;
			case rightward:
				m_rightward = new node_t(key, key_length, value);
				break;
			}
			return 0;
		}
		uint32_t remove_internal(const key_type_name* key, size_t key_length, bool& remove_me) {
			// The remove operation is sui generis in the sense that
			// it needes a sort of m_backward (a reverse) pointer.
			// Since this reverse pointer is not useful at any other occasion,
			// it is not worth keeping a permanent reference on it, so the
			// temporary recursive stack reference is enough.
			remove_me = false;
			uint32 result = 0;
			if (*key == m_key_atom) {
				--key_length;
				if (0 == key_length) {
					if (m_assigned) {
						m_assigned = false;
					} else {
						result = error_key_not_found;
					}
				} else {
					if (m_downward) {
						++key;
						result = m_downward->remove_internal(key, key_length, remove_me);
						if (0 == result && remove_me) {
							delete m_downward;
							m_downward = nullptr;
							/*
							+ac:    +ae     +ab     +ad        +af            -ac     
							    a      a       a       a          a              a
							    |      |       |       |          |              |
							    c      c       c       c          c              d
							            \     / \     / \        / \            / \
							             e   b   e   b   \      b   \          b   e
							                              \	         \              \
							                               \          \              f
							                                e          e
							                               /          / \
							                              d	         d   f

                                   d                  d      
                                   |                  |      
                                   d                  d      
                                  / \                / \     
                                 /   \     -dd      /   \    
                                /     \            /     \   
                               b       f          b       f  
                              / \     / \        / \     / \ 
                             a   c   e   g      a   c   e   g
                                                         \
                             e.left is null by definition, because the next node just below the top d is the leftmost to d's right.
                             e.left becomes equal to the left of the removed d
                             e.right becomes equal to the right of the removed d
                             f.left becomes the right of the moved e
                             e becomes the bottom of the top d

                             So, if a node is not assigned and has no downward node it can safely be
							removed but the tree must be rearranged.
							How to do this?????
							*/
							if (!m_assigned) {

							}
						}
					}
				}
			} else {
				if (*key > m_key_atom) {
					if (m_rightward) {
						result = m_rightward->remove_internal(key, key_length, remove_me);
						if (0 == result && remove_me) {
							delete m_rightward;
							m_rightward = nullptr;
						}
					}
				} else {
					if (m_leftward) {
						result = m_leftward->remove_internal(key, key_length, remove_me);
						if (0 == result && remove_me) {
							delete m_leftward;
							m_leftward = nullptr;
						}
					}
				}
			}
			remove_me = (0 == result && !m_assigned && !m_leftward && !m_downward && !m_rightward);
			return result;
		}
	};
	// 308 (pagina do livro)
	node_p m_root_node;
};
#endif
