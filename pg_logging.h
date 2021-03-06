#ifndef PG_LOGGING_H
#define PG_LOGGING_H

#include "postgres.h"
#include "pg_config.h"
#include "storage/lwlock.h"
#include "utils/timestamp.h"

#define CHECK_DATA

#if PG_VERSION_NUM < 90600
#error "pg_logging support only postgres starting from 9.6"
#endif

typedef enum ItemObjectType {
	IOT_NONE,
	IOT_TABLE,
	IOT_COLUMN,
	IOT_DATATYPE,
	IOT_CONSTRAINT
} ItemObjectType;

/* CollectedItem contains offsets in saved block */
typedef struct CollectedItem
{
#ifdef CHECK_DATA
	int			magic;
#endif
	int			totallen;		/* size of this block */

	TimestampTz	logtime;
	TimestampTz session_start_time;

	int			elevel;			/* error level */
	int			saved_errno;	/* errno at entry */
	int			sqlerrcode;		/* encoded ERRSTATE */

	/* text lengths in data block */
	int			message_len;
	int			detail_len;			/* errdetail */
	int			detail_log_len;
	int			hint_len;			/* errhint */
	int			context_len;
	int			domain_len;
	int			context_domain_len;
	int			command_tag_len;
	int			remote_host_len;
	int			errstate_len;

	/* query data */
	int			query_len;
	int			query_pos;
	int			internalpos;
	int			internalquery_len;

	/* extra data */
	int			ppid;
	int			appname_len;
	Oid			database_id;
	int			user_id;
	uint64		log_line_number;

	/* transaction info */
	int				vxid_len;
	TransactionId	txid;

	/* texts are contained here */
	char		data[FLEXIBLE_ARRAY_MEMBER];
} CollectedItem;

#define ITEM_HDR_LEN (offsetof(CollectedItem, data))

typedef struct LoggingShmemHdr
{
	char			   *data;
	volatile uint32		readpos;
	volatile uint32		endpos;
	int					buffer_size;			/* total size of buffer */
	int					buffer_size_initial;	/* initial size of buffer */
	LWLockPadded		hdr_lock;
	bool				wraparound;

	/* gucs */
	bool				logging_enabled;
	bool				ignore_statements;
	bool				set_query_fields;
	int					minlevel;
} LoggingShmemHdr;

#define HDR_LOCK() 	( LWLockAcquire(&hdr->hdr_lock.lock, LW_EXCLUSIVE) )
#define HDR_RELEASE() (	LWLockRelease(&hdr->hdr_lock.lock) )

struct ErrorLevel {
	char   *text;
	int		code;
};

#define	PG_LOGGING_MAGIC	0xAABBCCDD
#define	PG_ITEM_MAGIC		0x06054AB5

// view attributes, make sure it's equal to log_item type from sql
enum {
	Anum_pg_logging_logtime = 1,
	Anum_pg_logging_level,
	Anum_pg_logging_pid,
	Anum_pg_logging_line_num,
	Anum_pg_logging_appname,
	Anum_pg_logging_start_time,
	Anum_pg_logging_datid,
	Anum_pg_logging_errno,
	Anum_pg_logging_errcode,
	Anum_pg_logging_errstate,
	Anum_pg_logging_message,
	Anum_pg_logging_detail,
	Anum_pg_logging_detail_log,
	Anum_pg_logging_hint,
	Anum_pg_logging_context,
	Anum_pg_logging_context_domain,
	Anum_pg_logging_domain,
	Anum_pg_logging_internalpos,
	Anum_pg_logging_internalquery,
	Anum_pg_logging_userid,
	Anum_pg_logging_remote_host,
	Anum_pg_logging_command_tag,
	Anum_pg_logging_vxid,
	Anum_pg_logging_txid,
	Anum_pg_logging_query,
	Anum_pg_logging_query_pos,
	Anum_pg_logging_position,

	Natts_pg_logging_data
};

extern struct ErrorLevel errlevel_wordlist[];
extern LoggingShmemHdr	*hdr;

void reset_counters_in_shmem(int buffer_size);
struct ErrorLevel *get_errlevel (register const char *str, register size_t len);

#endif
