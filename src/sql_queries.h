#ifndef SQL_QUERIES_H
#define SQL_QUERIES_H

#include <QString>

namespace Queries
{
extern const QString kkrb_kgrbs;

//////////////////////////////////
//////////// MIGRATION ///////////
//////////////////////////////////
// TODO: сделать миграции
extern const QString create_table_migration;
extern const QString insert_migration;
extern const QString check_migration;

//////////////////////////////////
////////////// PLAN //////////////
//////////////////////////////////
extern const QString kgrbs_from_doc_plan;
extern const QString kubp_plan;
extern const QString kkrb_krp_plan;
extern const QString kkrb_kcs_plan;
extern const QString kkrb_kvr_plan;
extern const QString kkrb_kosgu_plan;
extern const QString notes_plan;
extern const QString num_from_plan_doc;
extern const QString insert_in_exp_plan;
extern const QString insert_in_exp_plan_manual;
extern const QString delete_in_exp_plan;
extern const QString insert_in_plan_data;
extern const QString delete_in_plan_data;
extern const QString delete_in_plan_data_documents;
extern const QString delete_in_plan_document;
extern const QString check_exp_plan_n_data;
extern const QString sum_row_exp_plan;
extern const QString sum_col_exp_plan;
extern const QString select_plan_document;
extern const QString select_plan_document_with_id;
extern const QString insert_plan_document;
extern const QString update_plan_document;
extern const QString check_kkrb_exp_plan;
extern const QString check_kosgu_with_notes;
extern const QString export_document_plan;
extern const QString export_data_plan;
extern const QString import_document_plan;
extern const QString import_data_plan;
extern const QString select_status_plan_create;
extern const QString select_all_bases;
extern const QString select_all_bases_scope;
extern const QString select_all_bases_edit;
extern const QString check_plan;
extern const QString check_inc_plan;
extern const QString check_id_plan;
extern const QString duplicate_plan_doc;
extern const QString duplicate_plan_data;
extern const QString check_num_plan_doc;
extern const QString solution_plan_without_kosgu;
extern const QString solution_plan_with_kosgu;
extern const QString insert_minus_kcs;
extern const QString insert_plus_kcs;
extern const QString insert_minus_krp_kcs;
extern const QString insert_plus_krp_kcs;
extern const QString check_plan_by_foreign_uid;
extern const QString delete_plan_doc_by_foreign_uid;
extern const QString delete_plan_data_by_foreign_uid;
extern const QString check_duplicate_exp_plan;
extern const QString insert_minus_ost_in_exp_plan;
extern const QString insert_minus_in_exp_plan;
extern const QString insert_merge_in_exp_plan;

//////////////////////////////////
////////////// FACT //////////////
//////////////////////////////////
extern const QString kubp_fact;
extern const QString kkrb_krp_fact;
extern const QString kkrb_kcs_fact;
extern const QString kkrb_kvr_fact;
extern const QString kkrb_kosgu_fact;
extern const QString notes_fact;
extern const QString exp_part_fact;
extern const QString direction_fact;
extern const QString num_from_fact_doc;
extern const QString insert_in_exp_fact;
extern const QString insert_in_exp_fact_manual;
extern const QString delete_in_exp_fact;
extern const QString insert_in_fact_data;
extern const QString delete_in_fact_data;
extern const QString delete_in_fact_document;
extern const QString check_exp_fact_n_data;
extern const QString balance_plan_n_fact;
extern const QString balance_fact_refund;
extern const QString sum_col_exp_fact;
extern const QString select_fact_document;
extern const QString select_fact_document_with_id;
extern const QString insert_fact_document;
extern const QString update_fact_document;
extern const QString check_kkrb_exp_fact;
extern const QString export_document_fact;
extern const QString export_data_fact;
extern const QString import_document_fact;
extern const QString import_data_fact;
extern const QString select_status_fact_create;
extern const QString duplicate_fact_doc;
extern const QString duplicate_fact_data;
extern const QString check_id_fact;
extern const QString check_num_fact_doc;
extern const QString check_fact_by_foreign_uid;
extern const QString delete_fact_doc_by_foreign_uid;
extern const QString delete_fact_data_by_foreign_uid;
extern const QString check_duplicate_exp_fact;
extern const QString get_type_doc_fact;
extern const QString insert_merge_in_exp_fact;
extern const QString insert_minus_in_exp_fact;

//////////////////////////////////
//////////// ANALITICS ///////////
//////////////////////////////////
extern const QString sum_analitic_plan_data;
extern const QString sum_analitic_plan_data_pivot;
extern const QString sum_analitic_plan_data_document_pivot;
extern const QString sum_analitic_finance_year;
extern const QString sum_analitic_finance_month;
extern const QString sum_remnants_year;
extern const QString sum_remnants_incrementally;
extern const QString sum_remnants_incrementally_year;
extern const QString sum_remnants_incrementally_period;
extern const QString remnants_incrementally;
extern const QString remnants_incrementally_current;
extern const QString drop_remnants_incrementally_period;
extern const QString create_remnants_incrementally_period;

//////////////////////////////////
///////////// UPDATE /////////////
//////////////////////////////////
extern const QString delete_statuses;
extern const QString insert_statuses;
extern const QString delete_kubp;
extern const QString insert_kubp;
extern const QString delete_kgrbs;
extern const QString insert_kgrbs;
extern const QString delete_krp;
extern const QString insert_krp;
extern const QString delete_kcs;
extern const QString insert_kcs;
extern const QString delete_kvr;
extern const QString insert_kvr;
extern const QString delete_kosgu;
extern const QString insert_kosgu;
extern const QString delete_bases;
extern const QString insert_bases;
extern const QString delete_bases_scope;
extern const QString insert_bases_scope;
extern const QString delete_plan_doc;
extern const QString delete_plan_data;
extern const QString insert_plan_doc;
extern const QString delete_fact_doc;
extern const QString delete_fact_data;
}  // namespace Queries

#endif  // SQL_QUERIES_H
