#include "rb_tree.h"
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

typedef void (*what_to_do)(struct rb_node *node);

what_to_do proc_node;
struct rb_tree *this_rb;


//struct rb_node* rb_alloc_new(_key_t key, ap_callback fn, void *arg);
int rb_insert(_key_t key, ap_callback fn, void *arg, ap_key* retval);
void rb_destroy();
int rb_delete(ap_key* key, int how);
void rb_print();
struct rb_node* rb_search(_key_t key);
int rb_del_in_range(_key_t to);

void rb_initial(struct rb_tree *t)
{
	t->num = 0;
	t->root = NULL;
	//t->rb_alloc_new = &rb_alloc_new;
	t->rb_insert = &rb_insert;
	t->rb_destroy = &rb_destroy;
	t->rb_delete = &rb_delete;
	t->rb_print = &rb_print;
	t->rb_search = &rb_search;
	t->rb_del_in_range = &rb_del_in_range;
	this_rb = t;
}


int _rb_compare(_key_t key1, _key_t key2)
{
	if(key1 > key2){
		return 1;
	}else if(key1 < key2){
		return -1;
	}
	return 0;
}


struct rb_node* _rb_search(_key_t key)// 
{
	struct rb_node *ptr = this_rb->root, *parent = NULL;
	int ret;
	while(ptr){
		if((ret = _rb_compare(key, ptr->key)) == 0){
			return ptr;
		}else if(ret > 0){
			parent = ptr;
			ptr = ptr->rb_right;
		}else{
			parent = ptr;
			ptr = ptr->rb_left;
		}
	}
	return parent;//if NULL, means the rbtree is empty.
}

struct rb_node* rb_search(_key_t key)
{
	struct rb_node *ptr = _rb_search(key);
	return _rb_compare(key, ptr->key) == 0 ? ptr : NULL;
}


/*
 * gaudent algorithm
 * only 1 bit of @x is 1. return the index of this bit.
 * e.g.
 * 0x00000400 --> 10
 */
int _ntz(unsigned int x)
{
	int b4,b3,b2,b1,b0;
	b4 = (x & 0x0000ffff)? 0:16;
	b3 = (x & 0x00ff00ff)? 0:8;
	b2 = (x & 0x0f0f0f0f)? 0:4;
	b1 = (x & 0x33333333)? 0:2;
	b0 = (x & 0x55555555)? 0:1;
	return b0 + b1 + b2 + b3 + b4;
}



/*
 * get an id to use
 */
int _getidx(unsigned int mask)
{
	if(~mask == 0){
		return -1;//FULL
	}
	unsigned int x = mask | (mask + 1);//first bit 0 ==> 1
	x = x & (~mask);
/*
	if(x >> 16){ x = x >> 16; id += 16; }
	if(x >> 8){ x = x >> 8; id += 8; }
	if(x >> 4){ x = x >> 4; id += 4; }
	id += (x >> 1) - (x >> 3);
*/
	return _ntz(x);
}

/*
 *
 * iterate mask's 1_bits and run functions.
 * 
 */
void _itr_mask(unsigned int mask, ap_callback fn[], void *arg[])
{
	unsigned int x;
	int id;
	while(mask != 0){
		x = mask & (mask - 1);//first bit 1 ==> 0
		x = (~x) & mask;
		//printf("%d %x\n",_ntz(x), mask);
		id = _ntz(x);

		//run fn.
		if(fn[id])
			fn[id](arg[id]);

		mask &= (~x);
	}
}



//if a new key
struct rb_node* _rb_alloc_new(_key_t key, ap_callback fn, void *arg)
{
	struct rb_node *temp = malloc(sizeof(struct rb_node));
	memset(temp, 0, sizeof(struct rb_node));
	temp->key = key;
	int i = _getidx(temp->mask);

	temp->fn[i] = fn;
	temp->arg[i] = arg;
	temp->mask |= (1<<i);
	return temp;
}

void _rb_rotate_right(struct rb_node *org_subroot)
{
	struct rb_node *y = org_subroot->rb_left;
	//y is not NULL
	org_subroot->rb_left = y->rb_right;
	if(org_subroot->rb_left){
		org_subroot->rb_left->rb_parent = org_subroot;
	}
	y->rb_parent = org_subroot->rb_parent;
	if(org_subroot->rb_parent){
		if(org_subroot == org_subroot->rb_parent->rb_left){
			org_subroot->rb_parent->rb_left = y;
		}else{
			org_subroot->rb_parent->rb_right = y;
		}
	}else{
		this_rb->root = y;
	}
	y->rb_right = org_subroot;
	org_subroot->rb_parent = y;
}

void _rb_rotate_left(struct rb_node *org_subroot)
{
	struct rb_node *y = org_subroot->rb_right;
	org_subroot->rb_right = y->rb_left;
	if(org_subroot->rb_right){
		org_subroot->rb_right->rb_parent = org_subroot;
	}
	y->rb_parent = org_subroot->rb_parent;
	if(org_subroot->rb_parent){
		if(org_subroot == org_subroot->rb_parent->rb_left){
			org_subroot->rb_parent->rb_left = y;
		}else{
			org_subroot->rb_parent->rb_right = y;
		}
	}else{
		this_rb->root = y;
	}
	y->rb_left = org_subroot;
	org_subroot->rb_parent = y;
}

void _rb_insert_fix(struct rb_node *node)
{
	struct rb_node *parent = node->rb_parent, *uncle;
	while(parent && (!parent->color)){
		//parent.color = 0 red
		if(parent == parent->rb_parent->rb_left){
			uncle = parent->rb_parent->rb_right;
			if(uncle && (!uncle->color)){
				parent->color = uncle->color = 1;
				parent->rb_parent->color = 0;
				node = parent->rb_parent;
				parent = node->rb_parent;
				continue;
			}else{
				//uncle is black 1
				if(node == parent->rb_left){
					parent->color = 1;
					parent->rb_parent->color = 0;
					_rb_rotate_right(parent->rb_parent);
					break;
				}else{
					_rb_rotate_left(parent);
					node = parent;
					parent = node->rb_parent;
					continue;
				}
			}
		}else{
			uncle = parent->rb_parent->rb_left;
			if(uncle && (!uncle->color)){
				parent->color = uncle->color = 1;
				parent->rb_parent->color = 0;
				node = parent->rb_parent;
				parent = node->rb_parent;
				continue;
			}else{
				if(node == parent->rb_right){
					parent->color = 1;
					parent->rb_parent->color = 0;
					_rb_rotate_left(parent->rb_parent);
					break;
				}else{
					_rb_rotate_right(parent);
					node = parent;
					parent = node->rb_parent;
					continue;
				}
			}
		}
	}
}

void _rb_travel_rbt(int mode, struct rb_node *ptr)//recursive travel rbtree
{
	if(ptr){
		switch(mode){
			case 0:
				proc_node(ptr);
				_rb_travel_rbt(0, ptr->rb_left);
				_rb_travel_rbt(0, ptr->rb_right);
				break;
			case 1:
				_rb_travel_rbt(1, ptr->rb_left);
				proc_node(ptr);
				_rb_travel_rbt(1, ptr->rb_right);
				break;
			case 2:
				_rb_travel_rbt(2, ptr->rb_left);
				_rb_travel_rbt(2, ptr->rb_right);
				proc_node(ptr);
				break;
		}
	}
}




int _rb_node_add(struct rb_node *node, ap_callback fn, void *arg)
{
	assert(node != NULL);
	int i = _getidx(node->mask);
	if(-1 == i){
		return -1;
	}

	node->fn[i] = fn;
	node->arg[i] = arg;
	node->mask |= (1<<i);

	return i;
}


/*
 * if key not in rbtree, alloc a node and insert
 * if key already in rbtree,  insert into one of this node's 32-buckets
 *
 * return - 0 succeess
 *		  - -1 fail 
 */
int rb_insert(_key_t key, ap_callback fn, void *arg, ap_key* retval)
{
	struct rb_node *ptr = _rb_search(key);
	struct rb_node *tmp;
	int id = -1;

	int ret;
	if(NULL == ptr){
		tmp = _rb_alloc_new(key, fn, arg);
		this_rb->root = tmp;
		goto out;//retval 0 tmp
	}
	if((ret = _rb_compare(key, ptr->key)) == 0){
		//already in rbtree
		if(-1 == (id = _rb_node_add(ptr, fn, arg))){
			return -1;
		}
		goto out;//retval id ptr
	}else{
		tmp = _rb_alloc_new(key, fn, arg);
		if(ret > 0){
			ptr->rb_right = tmp;//node->color == 0 red
		}else{
			ptr->rb_left = tmp;
		}
		tmp->rb_parent = ptr;
		_rb_insert_fix(tmp);
		//retval 0 tmp
	}
out:
	this_rb->root->color = 1;
	this_rb->num++;
	if(retval && -1 == id){
		retval->i = 0;
		retval->node_ptr = tmp;
	}else if(retval){//
		retval->i = id;
		retval->node_ptr = ptr;
	}
	return 0;
}

void _rb_print_node(struct rb_node *node)
{
	printf("%lld\n",(node->key));
}

//void _rb_freekey(_key_t key){
//	
//	printf("key: %d free!\n",*key);
//	free(key);
//}


void _rb_destroy_node(struct rb_node *node)
{
	free(node);
}

void rb_print()
{
	proc_node = &_rb_print_node;
	_rb_travel_rbt(0, this_rb->root);
}

void rb_destroy()
{
	proc_node = &_rb_destroy_node;
	_rb_travel_rbt(2, this_rb->root);
}

struct rb_node * _find_min(struct rb_node *node)
{
	while(node->rb_left){
		node = node->rb_left;
	}
	return node;
}

/*
 * return node's succeed
 * 
 */
struct rb_node * _find_succ(struct rb_node *node)
{
	if(node->rb_right){
		return _find_min(node->rb_right);
	}
	struct rb_node *parent = node->rb_parent;
	while(parent && (node == node->rb_parent->rb_right)){
		node = parent;
		parent = node->rb_parent;
	}
	return parent;//if NULL means no succ
}

/*
 * delete fix
 */
void _rb_del_fix(struct rb_node *node, struct rb_node *pa)
{
	struct rb_node *parent = pa;
	while( (!node || node->color) && node != this_rb->root){
		if(node == parent->rb_left){
			struct rb_node *bro = parent->rb_right;
			if(bro && 0 == bro->color){//bro is not NULL
				bro->color = 1;
				parent->color = 0;
				_rb_rotate_left(parent);
				bro = parent->rb_right;//continue 
			}
			if((!bro->rb_left || bro->rb_left->color) && (!bro->rb_right || bro->rb_right->color)){
				bro->color = 0;
				node = parent;
				parent = node->rb_parent;
			}else{//one red in two of bro's childs
				if(!bro->rb_right || bro->rb_right->color){//
					if(bro->rb_left)
						bro->rb_left->color = 1;
					bro->color = 0;
					_rb_rotate_right(bro);
					bro = parent->rb_right;
				}
				bro->color = parent->color;
				parent->color = 1;
				if(bro->rb_right)
					bro->rb_right->color = 1;
				_rb_rotate_left(parent);
				node = this_rb->root;
			}
		}else{
			struct rb_node *bro = parent->rb_left;
			if(bro && 0 == bro->color){
				bro->color = 1;
				parent->color = 0;
				_rb_rotate_right(parent);
				bro = parent->rb_left;
			}
			if((!bro->rb_left || bro->rb_left->color) && (!bro->rb_right || bro->rb_right->color)){
				bro->color = 0;
				node = parent;
				parent = node->rb_parent;
			}else{
				if(!bro->rb_left || bro->rb_left->color){
					if(bro->rb_right)
						bro->rb_right->color = 1;
					bro->color = 0;
					_rb_rotate_left(bro);
					bro = parent->rb_left;
				}
				bro->color = parent->color;
				parent->color = 1;
				if(bro->rb_left)
					bro->rb_left->color = 1;
				_rb_rotate_right(parent);
				node = this_rb->root;
			}
		}
	}
	if(node)
		node->color = 1;
}


void _rb_del_node(struct rb_node *node)
{
	assert(node != NULL);
	struct rb_node *rel_del_ptr;
	if(node->rb_left == NULL || node->rb_right == NULL){
		rel_del_ptr = node;
	}else{
		rel_del_ptr = _find_succ(node);
	}
	struct rb_node *del_child = rel_del_ptr->rb_right ? rel_del_ptr->rb_right : rel_del_ptr->rb_left;//can be null
	if(del_child){
		del_child->rb_parent = rel_del_ptr->rb_parent;
	}

	if(!rel_del_ptr->rb_parent){
		this_rb->root = del_child;
	}else{
		if(rel_del_ptr == rel_del_ptr->rb_parent->rb_right){
			rel_del_ptr->rb_parent->rb_right = del_child;
		}else{
			rel_del_ptr->rb_parent->rb_left = del_child;
		}
	}
	if(rel_del_ptr != node){
		node->key = rel_del_ptr->key;
	}

	if(rel_del_ptr->color){
		//black, fix sth.
		_rb_del_fix(del_child, rel_del_ptr->rb_parent);
	}
	_rb_destroy_node(rel_del_ptr);
}


/*
 * run all the callbacks and delete the node.
 * - or -
 * delete only the bucket(idx == key->i) , if all buckets of the node are empty, delete the node.
 */
int rb_delete(ap_key* key, int how)
{
	assert(key != NULL);
	if(how == DEL_RUNALL_CB){//run all callback, and delete it;
		_itr_mask(key->node_ptr->mask, key->node_ptr->fn, key->node_ptr->arg);
		key->node_ptr->mask = 0;
	}else if(how == DEL_ONLY){
		int x = 1<<(key->i);
		if( key->node_ptr->mask & x){
			key->node_ptr->mask &= ~x;
		}
	}
	if(0 == key->node_ptr->mask){
		_rb_del_node(key->node_ptr);
	}
	return how;
}



/*
 * find all node whose key is in range [MIN, to]
 * return number of nodes.
 * 
 * 
 */
int rb_del_in_range(_key_t to)//[MIN, to]
{
	struct rb_node *min_ptr = _find_min(this_rb->root), *ptr;
	ap_key tmp;
	int num = 0;
	while(min_ptr && min_ptr->key <= to){
		ptr = _find_succ(min_ptr);

		tmp.node_ptr = min_ptr;
		rb_delete(&tmp, DEL_RUNALL_CB);

		min_ptr = ptr;
		num++;
	}
	return num;
}