#ifndef DFC_H
#define DFC_H
/*
 * dfc.h
 *
 * header file for dfc.c
 */
#define VERSION "1.1.0"

/*
 * structure needed to store informations about mounted fs
 * It should contain brut datas.
 * Later on, we would need to compute those infos:
 * unsigned long bused;	used blocks
 * int prct_used;	usage of fs in percent
 * int prct_free;	free space of fs in percent
 */
struct fsmntinfo {
	/* infos to get from getmntent(3) */
	char *fsname;	/* name of mounted file system */
	char *dir;	/* file system path prefix */
	char *type;	/* mount type */

	/* infos to get from statvfs(3) */
	unsigned long bsize;	/* file system block size */
	unsigned long blocks;	/* size of fs */
	unsigned long bfree;	/* free blocks */

	/* pointer to the next element of the list */
	struct fsmntinfo *next;
};

/* list structure to store fsmntinfo */
struct list {
	struct fsmntinfo *head;
	struct fsmntinfo *tail;

	int fsmaxlen; /* should be the size of the longest fsname */
	int dirmaxlen; /* same for dir */
	int typemaxlen; /* same for type */
};

/* function declaration */
static void usage(int status);
void init_queue(struct list *lst);
int is_empty(struct list lst);
int enqueue(struct list *lst, struct fsmntinfo elt);
struct fsmntinfo fmi_init(void);
void disp(struct list lst);
void disp_header(struct list *lst);
double cvrt(double nb);
int imax(int a, int b);
char* trk(char *str);

#endif /* ndef DFC_H */
