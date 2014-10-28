

struct table;
struct object {
	int id;
};

struct table *table_create();
void table_release(struct table *t);
void table_insert(struct table *t,int id,struct object *obj);
struct object *table_delete(struct table *t,int id);
struct object *table_find(struct table *t,int id);
