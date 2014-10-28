#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "table.h"

struct slot {
	int id;
	struct object *obj;
	int next;
};

struct table {
	int size;
	int lastfree;
	struct slot *slot;
};


void 
rehash(struct table *t) {
	struct slot *oslot = t->slot;
	int osize = t->size;
	t->size = 2 * osize;
	t->lastfree = t->size - 1;
	t->slot = malloc(t->size * sizeof(struct slot));
	
	int i;
	for (i = 0; i < t->size; i++) {
		struct slot *s = &t->slot[i];
		s->id = -1;
		s->obj = NULL;
		s->next = -1;
	}
	
	for (i = 0; i < osize; i++) {
		struct slot * s = &oslot[i];
		if (s->obj)
			table_insert(t, s->id, s->obj);
	}
	free(oslot);
}

struct table*
table_create() {
	struct table *t = malloc(sizeof(*t));
	t->size = 64;
	t->lastfree = t->size - 1;
	t->slot = malloc(t->size * sizeof(struct slot));

	int i = 0;
	for(;i < t->size;++i) {
		struct slot *s = &t->slot[i];
		s->id = -1;
		s->obj = 0;
		s->next = -1;
	}
	return t;
}

void
table_release(struct table *t) {
	free(t->slot);
	free(t);
}

struct slot*
mainposition(struct table *t,int id) {
	int hash = id & (t->size - 1);
	return &t->slot[hash];
}

void 
table_insert(struct table *t,int id,struct object *obj) {
	struct slot *s = mainposition(t,id);
	if (s->obj == NULL) {
		s->id = id;
		s->obj = obj;
		return;
	}

	if (mainposition(t,s->id) != s) {
		struct slot *last = mainposition(t,s->id);
		while (last->next != s-t->slot) {
			last = &t->slot[last->next];
		}
		int temp_id = s->id;
		struct object *temp_obj = s->obj;

		last->next = s->next;
		s->id = id;
		s->obj = obj;
		s->next = -1;
		if (temp_obj)
			table_insert(t,temp_id,temp_obj);
		return;
	}

	while(t->lastfree >= 0) {
		struct slot *tmp = &t->slot[t->lastfree--];
		if (tmp->id == -1 && tmp->next != -1) {
			tmp->id = id;
			tmp->obj = obj;
			tmp->next = s->next;
			s->next = (int)(tmp-t->slot);
			return;
		}
	}
	rehash(t);
}

struct object*
table_delete(struct table *t,int id) {
	int hash = id & (t->size-1);
	struct slot *s = &t->slot[hash];
	struct slot *next = s;
	for(;;) {
		if (next->id == id) {
			struct object *obj = next->obj;
			next->obj = NULL;
			if (next != s) {
				s->next = next->next;
				next->next = -1;
				if (t->lastfree < next - t->slot) 
					t->lastfree = next - t->slot;
			}
			else {
				if (next->next != -1) {
					struct slot *nextslot = &t->slot[next->next];
					next->id = nextslot->id;
					next->obj = nextslot->obj;
					next->next = nextslot->next;
					nextslot->id = 0;
					nextslot->obj = 0;
					nextslot->next = -1;
				}
			}
			return obj;
		}
		if (next->next < 0)
			return NULL;

		s = next;
		next = &t->slot[next->next];
	}
	return NULL;
}

struct object*
table_find(struct table *t,int id) {
	struct slot *s = mainposition(t,id);
	for(;;) {
		if (s->id == id) 
			return s->obj;
		
		if (s->next == -1)
			break;
		s = &t->slot[s->next];
	}
	return NULL;
}
