/*
 * Note: this file originally auto-generated by mib2c
 * using mfd-data-get.m2c
 *
 * @file hmStyleTable_data_get.h
 *
 * @addtogroup get
 *
 * Prototypes for get functions
 *
 * @{
 */
#ifndef HMSTYLETABLE_DATA_GET_H
#define HMSTYLETABLE_DATA_GET_H

#ifdef __cplusplus
extern "C" {
#endif

/* *********************************************************************
 * GET function declarations
 */

/* *********************************************************************
 * GET Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table hmStyleTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * HOST-MUSIC-MIB::hmStyleTable is subid 1 of hmStyle.
 * Its status is Current.
 * OID: .1.3.6.1.3.2020.2.1, length: 8
*/
    /*
     * indexes
     */

    int hmStyleDescr_get( hmStyleTable_rowreq_ctx *rowreq_ctx, char **hmStyleDescr_val_ptr_ptr, size_t *hmStyleDescr_val_ptr_len_ptr );


int hmStyleTable_indexes_set_tbl_idx(hmStyleTable_mib_index *tbl_idx, long hmStyleIndex_val);
int hmStyleTable_indexes_set(hmStyleTable_rowreq_ctx *rowreq_ctx, long hmStyleIndex_val);




#ifdef __cplusplus
}
#endif

#endif /* HMSTYLETABLE_DATA_GET_H */
/** @} */
