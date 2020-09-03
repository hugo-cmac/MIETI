/*
 * Note: this file originally auto-generated by mib2c
 * using mfd-top.m2c
 */
#ifndef HMFORMATTABLE_H
#define HMFORMATTABLE_H

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
config_require(HOST-MUSIC-MIB/hmFormatTable/hmFormatTable_interface)
config_require(HOST-MUSIC-MIB/hmFormatTable/hmFormatTable_data_access)
config_require(HOST-MUSIC-MIB/hmFormatTable/hmFormatTable_data_get)
config_require(HOST-MUSIC-MIB/hmFormatTable/hmFormatTable_data_set)
    /* *INDENT-ON*  */

/* OID and column number definitions for hmFormatTable */
#include "hmFormatTable_oids.h"

/* enum definions */
#include "hmFormatTable_enums.h"

/* *********************************************************************
 * function declarations
 */
void init_hmFormatTable(void);
void shutdown_hmFormatTable(void);

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table hmFormatTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * HOST-MUSIC-MIB::hmFormatTable is subid 1 of hmFormat.
 * Its status is Current.
 * OID: .1.3.6.1.3.2020.5.1, length: 8
*/
/* *********************************************************************
 * When you register your mib, you get to provide a generic
 * pointer that will be passed back to you for most of the
 * functions calls.
 *
 * TODO:100:r: Review all context structures
 */
    /*
     * TODO:101:o: |-> Review hmFormatTable registration context.
     */
typedef netsnmp_data_list hmFormatTable_registration;

/**********************************************************************/
/*
 * TODO:110:r: |-> Review hmFormatTable data context structure.
 * This structure is used to represent the data for hmFormatTable.
 */
/*
 * This structure contains storage for all the columns defined in the
 * hmFormatTable.
 */
typedef struct hmFormatTable_data_s {
    
        /*
         * hmFormatDescr(2)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
   char   hmFormatDescr[255];
size_t      hmFormatDescr_len; /* # of char elements, not bytes */
    
} hmFormatTable_data;


/*
 * TODO:120:r: |-> Review hmFormatTable mib index.
 * This structure is used to represent the index for hmFormatTable.
 */
typedef struct hmFormatTable_mib_index_s {

        /*
         * hmFormatIndex(1)/INTEGER32/ASN_INTEGER/long(long)//l/A/w/e/R/d/h
         */
   long   hmFormatIndex;


} hmFormatTable_mib_index;

    /*
     * TODO:121:r: |   |-> Review hmFormatTable max index length.
     * If you KNOW that your indexes will never exceed a certain
     * length, update this macro to that length.
*/
#define MAX_hmFormatTable_IDX_LEN     1


/* *********************************************************************
 * TODO:130:o: |-> Review hmFormatTable Row request (rowreq) context.
 * When your functions are called, you will be passed a
 * hmFormatTable_rowreq_ctx pointer.
 */
typedef struct hmFormatTable_rowreq_ctx_s {

    /** this must be first for container compare to work */
    netsnmp_index        oid_idx;
    oid                  oid_tmp[MAX_hmFormatTable_IDX_LEN];
    
    hmFormatTable_mib_index        tbl_idx;
    
    hmFormatTable_data              data;

    /*
     * flags per row. Currently, the first (lower) 8 bits are reserved
     * for the user. See mfd.h for other flags.
     */
    u_int                       rowreq_flags;

    /*
     * TODO:131:o: |   |-> Add useful data to hmFormatTable rowreq context.
     */
    
    /*
     * storage for future expansion
     */
    netsnmp_data_list             *hmFormatTable_data_list;

} hmFormatTable_rowreq_ctx;

typedef struct hmFormatTable_ref_rowreq_ctx_s {
    hmFormatTable_rowreq_ctx *rowreq_ctx;
} hmFormatTable_ref_rowreq_ctx;

/* *********************************************************************
 * function prototypes
 */
    int hmFormatTable_pre_request(hmFormatTable_registration * user_context);
    int hmFormatTable_post_request(hmFormatTable_registration * user_context,
        int rc);

    int hmFormatTable_rowreq_ctx_init(hmFormatTable_rowreq_ctx *rowreq_ctx,
                                   void *user_init_ctx);
    void hmFormatTable_rowreq_ctx_cleanup(hmFormatTable_rowreq_ctx *rowreq_ctx);


    hmFormatTable_rowreq_ctx *
                  hmFormatTable_row_find_by_mib_index(hmFormatTable_mib_index *mib_idx);

extern const oid hmFormatTable_oid[];
extern const int hmFormatTable_oid_size;


#include "hmFormatTable_interface.h"
#include "hmFormatTable_data_access.h"
#include "hmFormatTable_data_get.h"
#include "hmFormatTable_data_set.h"

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

#endif /* HMFORMATTABLE_H */
/** @} */
