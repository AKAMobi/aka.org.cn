/*
 * Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
 */

#ifndef _LINUX_REISER_FS_SB
#define _LINUX_REISER_FS_SB

#ifdef __KERNEL__
#include <linux/tqueue.h>
#endif

#define UNSET_HASH 0 // read_super will guess about, what hash names
                     // in directories were sorted with
#define TEA_HASH  1
#define YURA_HASH 2
#define R5_HASH   3
#define DEFAULT_HASH R5_HASH

struct reiserfs_super_block
{
  __u32 s_block_count;			/* blocks count         */
  __u32 s_free_blocks;                  /* free blocks count    */
  __u32 s_root_block;           	/* root block number    */
  __u32 s_journal_block;           	/* journal block number    */
  __u32 s_journal_dev;           	/* journal device number  */
  __u32 s_orig_journal_size; 		/* size of the journal on FS creation.  used to make sure they don't overflow it */
  __u32 s_journal_trans_max ;           /* max number of blocks in a transaction.  */
  __u32 s_journal_block_count ;         /* total size of the journal. can change over time  */
  __u32 s_journal_max_batch ;           /* max number of blocks to batch into a trans */
  __u32 s_journal_max_commit_age ;      /* in seconds, how old can an async commit be */
  __u32 s_journal_max_trans_age ;       /* in seconds, how old can a transaction be */
  __u16 s_blocksize;                   	/* block size           */
  __u16 s_oid_maxsize;			/* max size of object id array, see get_objectid() commentary  */
  __u16 s_oid_cursize;			/* current size of object id array */
  __u16 s_state;                       	/* valid or error       */
  char s_magic[12];                     /* reiserfs magic string indicates that file system is reiserfs */
  __u32 s_hash_function_code;		/* indicate, what hash fuction is being use to sort names in a directory*/
  __u16 s_tree_height;                  /* height of disk tree */
  __u16 s_bmap_nr;                      /* amount of bitmap blocks needed to address each block of file system */
  __u16 s_reserved;
};

#define SB_SIZE (sizeof(struct reiserfs_super_block))

/* LOGGING -- */

/* These all interelate for performance.  
**
** If the journal block count is smaller than n transactions, you lose speed. I don't know what n is yet, I'm guessing 8-16 
** 
** If your journal fills faster than dirty buffers get flushed to disk, it must flush them before allowing the journal
** to wrap, which slows things down.  If you need high speed meta data updates, the journal should be big enough
** to prevent wrapping before dirty meta blocks get to disk.
**
** If the batch max is smaller than the transaction max, you'll waste space at the end of the journal
** because journal_end sets the next transaction to start at 0 if the next transaction has any chance of wrapping.
**
** The large the batch max age, the better the speed, and the more meta data changes you'll lose after a crash.
**
*/

/*
** transaction handle which is passed around for all journal calls
*/
struct reiserfs_transaction_handle {
  char *t_caller ;              /* debugging use */
  int t_blocks_logged ;         /* number of blocks this writer has actually logged */
  int t_blocks_allocated ;      /* number of blocks this writer allocated */
  unsigned long t_trans_id ;    /* sanity check, should equal the current trans id */
  struct super_block *t_super ; /* super for this FS when journal_begin was called. saves calls to reiserfs_get_super */
} ;

struct reiserfs_bitmap_node {
  int id ;
  char *data ;
  struct list_head list ;
} ;

struct reiserfs_list_bitmap {
  struct reiserfs_journal_list *journal_list ;
  struct reiserfs_bitmap_node **bitmaps ;
} ;

#define JOURNAL_TRANS_MAX 1024   /* biggest possible single transaction, don't change for now (8/3/99) */

#ifdef __KERNEL__


/* don't mess with these for a while */
#define JOURNAL_BLOCK_SIZE  4096 /* BUG gotta get rid of this */
#define JOURNAL_MAX_CNODE   1500 /* max cnodes to allocate. */
#define JOURNAL_HASH_SIZE 2048   /* size of journal hash table, used to index lookups in current commit  */
#define JOURNAL_LIST_HASH_SIZE 16384   
#define JOURNAL_NUM_BITMAPS 5 /* number of copies of the bitmaps to have floating.  Must be >= 2 */
#define JOURNAL_LIST_COUNT 128
#define BH_JDirty       16      /* journal data needs to be written before buffer can be marked dirty */
#define BH_JDirty_wait 18	/* commit is done, buffer marked dirty */
#define BH_JNew 19		/* buffer allocated during this transaction, no need to write if freed during this trans too */

/* One of these for every block in every transaction
** Each one is in two hash tables.  First, a hash of the current transaction, and after journal_end, a
** hash of all the in memory transactions.
** next and prev are used by the current transaction (journal_hash).
** hnext and hprev are used by journal_list_hash.  If a block is in more than one transaction, the journal_list_hash
** links it in multiple times.  This allows the end_io handler, and flush_journal_list to remove just the cnode belonging
** to a given transaction.
*/
struct reiserfs_journal_cnode {
  struct buffer_head *bh ;		 /* real buffer head */
  kdev_t dev ;				 /* dev of real buffer head */
  unsigned long blocknr ;		 /* block number of real buffer head, == 0 when buffer on disk */		 
  int state ;
  struct reiserfs_journal_list *jlist ;  /* journal list this cnode lives in */
  struct reiserfs_journal_cnode *next ;  /* next in transaction list */
  struct reiserfs_journal_cnode *prev ;  /* prev in transaction list */
  struct reiserfs_journal_cnode *hprev ; /* prev in hash list */
  struct reiserfs_journal_cnode *hnext ; /* next in hash list */
};


/*
** one of these for each transaction.  The most important part here is the j_realblock.
** this list of cnodes is used to hash all the blocks in all the commits, to mark all the
** real buffer heads dirty once all the commits hit the disk,
** and to make sure every real block in a transaction is on disk before allowing the log area
** to be overwritten
*/
struct reiserfs_journal_list {
  unsigned long j_start ;
  unsigned long j_len ;
  atomic_t j_nonzerolen ;
  atomic_t j_commit_left ;
  atomic_t j_flushing ;
  atomic_t j_commit_flushing ;
  atomic_t j_older_commits_done ;      /* all commits older than this on disk*/
  unsigned long j_trans_id ;
  time_t j_timestamp ;
  struct reiserfs_list_bitmap *j_list_bitmap ;
  struct buffer_head *j_commit_bh ; /* commit buffer head */
  struct reiserfs_journal_cnode *j_realblock  ;
  struct reiserfs_journal_cnode *j_freedlist ; /* list of buffers that were freed during this trans.  free each of these on flush */
  struct wait_queue *j_commit_wait ; /* wait for all the commit blocks to be flushed */
  struct wait_queue *j_flush_wait ; /* wait for all the real blocks to be flushed */
} ;


struct reiserfs_journal {
  struct buffer_head ** j_ap_blocks ; /* journal blocks on disk */
  struct reiserfs_journal_cnode *j_last ; /* newest journal block */
  struct reiserfs_journal_cnode *j_first ; /*  oldest journal block.  start here for traverse */
  int j_state ;			
  unsigned long j_trans_id ;
  unsigned long j_mount_id ;
  unsigned long j_start ;             /* start of current waiting commit (index into j_ap_blocks) */
  unsigned long j_len ;               /* lenght of current waiting commit */
  unsigned long j_len_alloc ;         /* number of buffers requested by journal_begin() */
  atomic_t j_wcount ;            /* count of writers for current commit */
  unsigned long j_bcount ;            /* batch count. allows turning X transactions into 1 */
  unsigned long j_first_unflushed_offset ;  /* first unflushed transactions offset */
  unsigned long j_last_flush_trans_id ;    /* last fully flushed journal timestamp */
  struct buffer_head *j_header_bh ;   
  time_t j_trans_start_time ;         /* time this transaction started */
  struct wait_queue *j_wait ;         /* wait  journal_end to finish I/O */
  atomic_t j_wlock ;                       /* lock for j_wait */
  struct wait_queue *j_join_wait ;    /* wait for current transaction to finish before starting new one */
  atomic_t j_jlock ;                       /* lock for j_join_wait */
  struct wait_queue *j_dobalance_wait ; /* call this before going into do_balance */
  struct wait_queue *j_commit_thread_wait ; /* commit thread waits on this between commits */
  struct wait_queue *j_commit_thread_done ; /* commit thread wakes this up when it has run its task queue */
  task_queue j_commit_thread_tq ;        /* task queue for this FS's commit thread */
  int j_dobalance_lock ;		/* lock for the above */
  int j_journal_list_index ;	      /* journal list number of the current trans */
  int j_list_bitmap_index ;	      /* number of next list bitmap to use */
  int j_must_wait ;		       /* no more journal begins allowed. MUST sleep on j_join_wait */
  int j_next_full_flush ;             /* next journal_end will flush all journal list */
  int j_next_async_flush ;             /* next journal_end will flush all async commits */

  int j_cnode_used ;	      /* number of cnodes on the used list */
  int j_cnode_free ;          /* number of cnodes on the free list */

  struct reiserfs_journal_cnode *j_cnode_free_list ;
  struct reiserfs_journal_cnode *j_cnode_free_orig ; /* orig pointer returned from vmalloc */

  int j_free_bitmap_nodes ;
  int j_used_bitmap_nodes ;
  struct list_head j_bitmap_nodes ;
  struct reiserfs_list_bitmap j_list_bitmap[JOURNAL_NUM_BITMAPS] ;	/* array of bitmaps to record the deleted blocks */
  struct reiserfs_journal_list j_journal_list[JOURNAL_LIST_COUNT] ;	    /* array of all the journal lists */
  struct reiserfs_journal_cnode *j_hash_table[JOURNAL_HASH_SIZE] ; 	    /* hash table for real buffer heads in current trans */ 
  struct reiserfs_journal_cnode *j_list_hash_table[JOURNAL_LIST_HASH_SIZE] ; /* hash table for all the real buffer heads in all 
  										the transactions */
};

#endif // __KERNEL__

#define JOURNAL_DESC_MAGIC "ReIsErLB" /* ick.  magic string to find desc blocks in the journal */



/*
 * reiserfs super block data in memory
 */

typedef __u32 (*hashf_t) (const char *, int);

struct reiserfs_sb_info
{
  struct buffer_head * s_sbh;                   /* Buffer containing the super block */
  struct reiserfs_super_block * s_rs;           /* Pointer to the super block in the buffer */
  struct buffer_head ** s_ap_true_bitmap;       /* array of buffers, holding block bitmap */
  struct reiserfs_journal *s_journal ;		/* pointer to journal information */
  unsigned short s_mount_state;                 /* reiserfs state (valid, invalid) */
  
  struct wait_queue * read_wait;                /* wait queue for reiserfs read lock */
  hashf_t s_hash_function;	/* pointer to function which is used
                                   to sort names in directory. Set on
                                   mount */
  unsigned long s_mount_opt;

  /* session statistics */
  int s_kmallocs;
  int s_disk_reads;
  int s_disk_writes;
  int s_fix_nodes;
  int s_do_balance;
  int s_unneeded_left_neighbor;
  int s_good_search_by_key_reada;
  int s_bmaps;
  int s_bmaps_without_search;
  int s_direct2indirect;
  int s_indirect2direct;

};


#ifdef __KERNEL__

#define NOTAIL 0  /* mount option -o notail */
#define GENERICREAD 2 /*          -o genericread */
#define REPLAYONLY 3 /* replay journal and return 0. Use by fsck */
#define NOLOG 4 /* don't log metadata at all.  */
#define FORCE_TEA_HASH 5
#define FORCE_RUPASOV_HASH 6
#define FORCE_R5_HASH 7
#define FORCE_HASH_DETECT 8


#define dont_have_tails(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << NOTAIL))
#define use_genericread(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << GENERICREAD))
#define replay_only(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << REPLAYONLY))
#define reiserfs_dont_log(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << NOLOG))
#define reiserfs_rupasov_hash(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << FORCE_RUPASOV_HASH))
#define reiserfs_tea_hash(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << FORCE_TEA_HASH))
#define reiserfs_r5_hash(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << FORCE_R5_HASH))
#define reiserfs_hash_detect(s) ((s)->u.reiserfs_sb.s_mount_opt & (1 << FORCE_HASH_DETECT))



void __wait_on_inode (struct inode * inode);
extern struct list_head inode_in_use;
extern spinlock_t inode_lock;
inline int  test_and_wait_on_buffer(struct buffer_head * p_s_bh);
struct buffer_head * reiserfs_get_hash_table (kdev_t  n_dev, int n_block, int n_size, int * p_n_repeat);
struct buffer_head * reiserfs_getblk (kdev_t  n_dev, int n_block, int n_size, int * p_n_repeat);
void reiserfs_show_buffers (kdev_t dev);
void fixup_reiserfs_buffers (kdev_t dev);
void reiserfs_refile_buffer (struct buffer_head * bh);
void reiserfs_file_buffer (struct buffer_head * bh, int list);
int reiserfs_is_super(struct super_block *s)  ;
int journal_mark_dirty(struct reiserfs_transaction_handle *, struct super_block *, struct buffer_head *bh) ;
int flush_old_commits(struct super_block *s, int) ;
int show_reiserfs_locks(void) ;
void reiserfs_end_buffer_io_sync (struct buffer_head *bh, int uptodate) ; 
void reiserfs_journal_end_io(struct buffer_head *bh, int uptodate) ;
int reiserfs_resize(struct super_block *, unsigned long) ;

#endif



#define SB_BUFFER_WITH_SB(s) ((s)->u.reiserfs_sb.s_sbh)
#define SB_DISK_SUPER_BLOCK(s) ((s)->u.reiserfs_sb.s_rs)
#define SB_JOURNAL(s) ((s)->u.reiserfs_sb.s_journal)
#define SB_JOURNAL_BLOCK(s) (SB_DISK_SUPER_BLOCK(s)->s_journal_block)
#define SB_JOURNAL_LIST(s) (SB_JOURNAL(s)->j_journal_list)
#define SB_JOURNAL_LIST_INDEX(s) (SB_JOURNAL(s)->j_journal_list_index) 
#define SB_JOURNAL_LEN_FREE(s) (SB_JOURNAL(s)->j_journal_len_free) 
#define SB_BLOCK_COUNT(s) (SB_DISK_SUPER_BLOCK(s)->s_block_count)
#define RESERVED_FOR_PRESERVE_LIST 500
#define SB_FREE_BLOCKS(s) (SB_DISK_SUPER_BLOCK(s)->s_free_blocks)
#define SB_REISERFS_MAGIC(s) (SB_DISK_SUPER_BLOCK(s)->s_magic)
#define SB_ROOT_BLOCK(s) (SB_DISK_SUPER_BLOCK(s)->s_root_block)
#define SB_TREE_HEIGHT(s) (SB_DISK_SUPER_BLOCK(s)->s_tree_height)
#define SB_REISERFS_STATE(s) (SB_DISK_SUPER_BLOCK(s)->s_state)
#define SB_BMAP_NR(s) (SB_DISK_SUPER_BLOCK(s)->s_bmap_nr)
#define SB_AP_BITMAP(s) ((s)->u.reiserfs_sb.s_ap_true_bitmap)

#endif	/* _LINUX_REISER_FS_SB */
