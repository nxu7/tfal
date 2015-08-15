#ifndef _RB_TREE_H
#define _RB_TREE_H



#define DEL_ONLY 10
#define DEL_RUNALL_CB 11
#define EACH_NODE_FNUM 32 


typedef void (* ap_callback)(void *);

typedef unsigned long long _key_t;//u64

struct rb_node{
	_key_t key; //
	struct rb_node *rb_left, *rb_right, *rb_parent;
	char color;// 0 - red, 1 - black

	ap_callback fn[EACH_NODE_FNUM];
	void *arg[EACH_NODE_FNUM];
 	unsigned int mask;
};

typedef struct _ap_key{
	struct rb_node *node_ptr;
	int i;
}ap_key;


struct rb_tree{
	int num;
	struct rb_node *root;


	/*some functions*/

	int ( *rb_insert)(_key_t key, ap_callback fn, void *arg, ap_key* retval);
	int ( *rb_delete)(ap_key* key,int how);
	int ( *rb_del_in_range)(_key_t to);
	void ( *rb_destroy)();
	void ( *rb_print)();
	struct rb_node* ( *rb_search)(_key_t key);
}rb_tree;

extern void rb_initial(struct rb_tree *t);

#endif