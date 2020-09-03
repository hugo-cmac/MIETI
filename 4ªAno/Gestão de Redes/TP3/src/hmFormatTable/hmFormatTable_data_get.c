/*
 * Note: this file originally auto-generated by mib2c
 * using mfd-data-get.m2c
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "hmFormatTable.h"


/** @defgroup data_get data_get: Routines to get data
 *
 * TODO:230:M: Implement hmFormatTable get routines.
 * TODO:240:M: Implement hmFormatTable mapping routines (if any).
 *
 * These routine are used to get the value for individual objects. The
 * row context is passed, along with a pointer to the memory where the
 * value should be copied.
 *
 * @{
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

/* ---------------------------------------------------------------------
 * TODO:200:r: Implement hmFormatTable data context functions.
 */


/**
 * set mib index(es)
 *
 * @param tbl_idx mib index structure
 * @param hmFormatIndex_val
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This convenience function is useful for setting all the MIB index
 *  components with a single function call. It is assume that the C values
 *  have already been mapped from their native/rawformat to the MIB format.
 */
int
hmFormatTable_indexes_set_tbl_idx(hmFormatTable_mib_index *tbl_idx, long hmFormatIndex_val)
{
    DEBUGMSGTL(("verbose:hmFormatTable:hmFormatTable_indexes_set_tbl_idx","called\n"));

    /* hmFormatIndex(1)/INTEGER32/ASN_INTEGER/long(long)//l/A/w/e/R/d/h */
    tbl_idx->hmFormatIndex = hmFormatIndex_val;
    

    return MFD_SUCCESS;
} /* hmFormatTable_indexes_set_tbl_idx */

/**
 * @internal
 * set row context indexes
 *
 * @param reqreq_ctx the row context that needs updated indexes
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This function sets the mib indexs, then updates the oid indexs
 *  from the mib index.
 */
int
hmFormatTable_indexes_set(hmFormatTable_rowreq_ctx *rowreq_ctx, long hmFormatIndex_val)
{
    DEBUGMSGTL(("verbose:hmFormatTable:hmFormatTable_indexes_set","called\n"));

    if(MFD_SUCCESS != hmFormatTable_indexes_set_tbl_idx(&rowreq_ctx->tbl_idx
                                   , hmFormatIndex_val
           ))
        return MFD_ERROR;

    /*
     * convert mib index to oid index
     */
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if(0 != hmFormatTable_index_to_oid(&rowreq_ctx->oid_idx,
                                    &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }

    return MFD_SUCCESS;
} /* hmFormatTable_indexes_set */


/*---------------------------------------------------------------------
 * HOST-MUSIC-MIB::hmFormatEntry.hmFormatDescr
 * hmFormatDescr is subid 2 of hmFormatEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.3.2020.5.1.1.2
 * Description:
Descricao do formato.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 1      hashint   1
 *   settable   0
 *   hint: 255a
 *
 * Ranges:  0 - 255;
 *
 * Its syntax is DisplayString (based on perltype OCTETSTR)
 * The net-snmp type is ASN_OCTET_STR. The C type decl is char (char)
 * This data type requires a length.  (Max 255)
 */
/**
 * Extract the current value of the hmFormatDescr data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param hmFormatDescr_val_ptr_ptr
 *        Pointer to storage for a char variable
 * @param hmFormatDescr_val_ptr_len_ptr
 *        Pointer to a size_t. On entry, it will contain the size (in bytes)
 *        pointed to by hmFormatDescr.
 *        On exit, this value should contain the data size (in bytes).
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
*
 * @note If you need more than (*hmFormatDescr_val_ptr_len_ptr) bytes of memory,
 *       allocate it using malloc() and update hmFormatDescr_val_ptr_ptr.
 *       <b>DO NOT</b> free the previous pointer.
 *       The MFD helper will release the memory you allocate.
 *
 * @remark If you call this function yourself, you are responsible
 *         for checking if the pointer changed, and freeing any
 *         previously allocated memory. (Not necessary if you pass
 *         in a pointer to static memory, obviously.)
 */
int
hmFormatDescr_get( hmFormatTable_rowreq_ctx *rowreq_ctx, char **hmFormatDescr_val_ptr_ptr, size_t *hmFormatDescr_val_ptr_len_ptr )
{
   /** we should have a non-NULL pointer and enough storage */
   netsnmp_assert( (NULL != hmFormatDescr_val_ptr_ptr) && (NULL != *hmFormatDescr_val_ptr_ptr));
   netsnmp_assert( NULL != hmFormatDescr_val_ptr_len_ptr );


    DEBUGMSGTL(("verbose:hmFormatTable:hmFormatDescr_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the hmFormatDescr data.
 * copy (* hmFormatDescr_val_ptr_ptr ) data and (* hmFormatDescr_val_ptr_len_ptr ) from rowreq_ctx->data
 */
    /*
     * make sure there is enough space for hmFormatDescr data
     */
    if ((NULL == (* hmFormatDescr_val_ptr_ptr )) ||
        ((* hmFormatDescr_val_ptr_len_ptr ) <
         (rowreq_ctx->data.hmFormatDescr_len* sizeof(rowreq_ctx->data.hmFormatDescr[0])))) {
        /*
         * allocate space for hmFormatDescr data
         */
        (* hmFormatDescr_val_ptr_ptr ) = malloc(rowreq_ctx->data.hmFormatDescr_len* sizeof(rowreq_ctx->data.hmFormatDescr[0]));
        if(NULL == (* hmFormatDescr_val_ptr_ptr )) {
            snmp_log(LOG_ERR,"could not allocate memory (rowreq_ctx->data.hmFormatDescr)\n");
            return MFD_ERROR;
        }
    }
    (* hmFormatDescr_val_ptr_len_ptr ) = rowreq_ctx->data.hmFormatDescr_len* sizeof(rowreq_ctx->data.hmFormatDescr[0]);
    memcpy( (* hmFormatDescr_val_ptr_ptr ), rowreq_ctx->data.hmFormatDescr, rowreq_ctx->data.hmFormatDescr_len* sizeof(rowreq_ctx->data.hmFormatDescr[0]) );

    return MFD_SUCCESS;
} /* hmFormatDescr_get */



/** @} */
