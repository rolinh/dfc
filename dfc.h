#ifndef DFC_H
#define DFC_H
/*
 * dfc.h
 *
 * header file for dfc.c
 */
#define VERSION "2.0.2"

/*
 * structure needed to store informations about mounted fs
 * It should contain brut datas.
 * Later on, we would need to compute those infos:
 * unsigned long bused;	used blocks
 * double prct_used;	usage of fs in percent
 * double prct_free;	free space of fs in percent
 */
struct fsmntinfo {
	/* infos to get from getmntent(3) */
	char *fsname;	/* name of mounted file system */
	char *dir;	/* file system path prefix */
	char *type;	/* mount type */
	char *opts;	/* mount options (see mntent.h) */

	/* infos to get from statvfs(3) */
	unsigned long bsize;	/* file system block size */
	unsigned long frsize;	/* fragment size */
	unsigned long blocks;	/* size of fs in frsize unit */
	unsigned long bfree;	/* # of free blocks */
	unsigned long bavail;	/* # of available blocks */
	unsigned long files;	/* # of inodes */
	unsigned long ffree;	/* # of free inodes */
	unsigned long favail;	/* # of available inodes */

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
void fetch_info(struct list *lst);
void disp(struct list *lst);
void disp_header(struct list *lst);
void disp_sum(struct list *lst, double stot, double utot, double ftot);
void disp_bar(double perct);
void disp_at(double avail, double total, double perct);
void disp_perct(double perct);
void change_color(double perct);
void reset_color(void);
double cvrt(double n);
double humanize(double n, double perct);
int imax(int a, int b);
char* trk(char *str);

#endif /* ndef DFC_H */
