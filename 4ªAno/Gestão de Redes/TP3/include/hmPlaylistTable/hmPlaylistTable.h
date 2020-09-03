/*
 * Note: this file originally auto-generated by mib2c
 * using mfd-top.m2c
 */
#ifndef HMPLAYLISTTABLE_H
#define HMPLAYLISTTABLE_H

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup misc misc: Miscellaneous routines
 *
 * @{
 */
#include <net-snmp/library/asn1.h>

/* other required module components */
    /* *INDENT-OFF*  */
config_add_mib(HOST-MUSIC-MIB)
config_require(HOST-MUSIC-MIB/hmPlaylistTable/hmPlaylistTable_interface)
config_require(HOST-MUSIC-MIB/hmPlaylistTable/hmPlaylistTable_data_access)
config_require(HOST-MUSIC-MIB/hmPlaylistTable/hmPlaylistTable_data_get)
config_require(HOST-MUSIC-MIB/hmPlaylistTable/hmPlaylistTable_data_set)
    /* *INDENT-ON*  */

/* OID and column number definitions for hmPlaylistTable */
#include "hmPlaylistTable_oids.h"

/* enum definions */
#include "hmPlaylistTable_enums.h"

/* *********************************************************************
 * function declarations
 */
void init_hmPlaylistTable(void);
void shutdown_hmPlaylistTable(void);

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table hmPlaylistTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * HOST-MUSIC-MIB::hmPlaylistTable is subid 1 of hmPlay.
 * Its status is Current.
 * OID: .1.3.6.1.3.2020.6.1, length: 8
*/
/* *********************************************************************
 * When you register your mib, you get to provide a generic
 * pointer that will be passed back to you for most of the
 * functions calls.
 *
 * TODO:100:r: Review all context structures
 */
    /*
     * TODO:101:o: |-> Review hmPlaylistTable registration context.
     */
typedef netsnmp_data_list hmPlaylistTable_registration;

/**********************************************************************/
/*
 * TODO:110:r: |-> Review hmPlaylistTable data context structure.
 * This structure is used to represent the data for hmPlaylistTable.
 */
/*
 * This structure contains storage for all the columns defined in the
 * hmPlaylistTable.
 */
typedef struct hmPlaylistTable_data_s {
    
} hmPlaylistTable_data;


/*
 * TODO:120:r: |-> Review hmPlaylistTable mib index.
 * This structure is used to represent the index for hmPlaylistTable.
 */
typedef struct hmPlaylistTable_mib_index_s {

        /*
         * hmPlaylistIndex(1)/INTEGER32/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
         */
   long   hmPlaylistIndex;


} hmPlaylistTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review hmPlaylistTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
*/
#define MAX_hmPlaylistTable_IDX_LEN     1


/* *********************************************************************
 * TODO:130:o: |-> Review hmPlaylistTable Row request (rowreq) context.
 * When your functions are called, you will be passed a
 * hmPlaylistTable_rowreq_ctx pointer.
 */
typedef struct hmPlaylistTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
    netsnmp_index        oid_idx;
    oid                  oid_tmp[MAX_hmPlaylistTable_IDX_LEN];
    
    hmPlaylistTable_mib_index        tbl_idx;
    
    hmPlaylistTable_data              data;

    /*
     * flags per row. Currently, the first (lower) 8 bits are reserved
     * for the user. See mfd.h for other flags.
     */
    u_int                       rowreq_flags;

    /*
     * TODO:131:o: |   |-> Add useful data to hmPlaylistTable rowreq context.
     */
    
    /*
     * storage for future expansion
     */
    netsnmp_data_list             *hmPlaylistTable_data_list;

} hmPlaylistTable_rowreq_ctx;

typedef struct hmPlaylistTable_ref_rowreq_ctx_s {
    hmPlaylistTable_rowreq_ctx *rowreq_ctx;
} hmPlaylistTable_ref_rowreq_ctx;

/* *********************************************************************
 * function prototypes
 */
    int hmPlaylistTable_pre_request(hmPlaylistTable_registration * user_context);
    int hmPlaylistTable_post_request(hmPlaylistTable_registration * user_context,
        int rc);

    int hmPlaylistTable_rowreq_ctx_init(hmPlaylistTable_rowreq_ctx *rowreq_ctx,
                                   void *user_init_ctx);
    void hmPlaylistTable_rowreq_ctx_cleanup(hmPlaylistTable_rowreq_ctx *rowreq_ctx);


    hmPlaylistTable_rowreq_ctx *
                  hmPlaylistTable_row_find_by_mib_index(hmPlaylistTable_mib_index *mib_idx);

extern const oid hmPlaylistTable_oid[];
extern const int hmPlaylistTable_oid_size;


#include "hmPlaylistTable_interface.h"
#include "hmPlaylistTable_data_access.h"
#include "hmPlaylistTable_data_get.h"
#include "hmPlaylistTable_data_set.h"

/*
 * DUMMY markers, ignore
 *
 * TODO:099:x: *************************************************************
 * TODO:199:x: *************************************************************
 * TODO:299:x: *************************************************************
 * TODO:399:x: *************************************************************
 * TODO:499:x: *************************************************************
 */

#ifdef __cplusplus
}
#endif

#endif /* HMPLAYLISTTABLE_H */
/** @} */
