DROP VIEW unpivot_expenses_editor_plan;

CREATE VIEW unpivot_expenses_editor_plan AS SELECT 
    pdb.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, expenses_part, p.notes, pdb.doc_number, pdb.doc_date,
    pdb.basis_type || CASE pdb.basis_number WHEN 0 THEN '' ELSE ' № ' || pdb.basis_number END AS title_document,
    Round(SUM(p.amount), 2) AS year_amount,
    Round(SUM(CASE WHEN month_n = 1 THEN p.amount ELSE 0 END), 2) AS m_1,
    Round(SUM(CASE WHEN month_n = 2 THEN p.amount ELSE 0 END), 2) AS m_2,
    Round(SUM(CASE WHEN month_n = 3 THEN p.amount ELSE 0 END), 2) AS m_3,
    Round(SUM(CASE WHEN month_n = 4 THEN p.amount ELSE 0 END), 2) AS m_4,
    Round(SUM(CASE WHEN month_n = 5 THEN p.amount ELSE 0 END), 2) AS m_5,
    Round(SUM(CASE WHEN month_n = 6 THEN p.amount ELSE 0 END), 2) AS m_6,
    Round(SUM(CASE WHEN month_n = 7 THEN p.amount ELSE 0 END), 2) AS m_7,
    Round(SUM(CASE WHEN month_n = 8 THEN p.amount ELSE 0 END), 2) AS m_8,
    Round(SUM(CASE WHEN month_n = 9 THEN p.amount ELSE 0 END), 2) AS m_9,
    Round(SUM(CASE WHEN month_n = 10 THEN p.amount ELSE 0 END), 2) AS m_10,
    Round(SUM(CASE WHEN month_n = 11 THEN p.amount ELSE 0 END), 2) AS m_11,
    Round(SUM(CASE WHEN month_n = 12 THEN p.amount ELSE 0 END), 2) AS m_12
FROM plan_data p
INNER JOIN
(
    SELECT DISTINCT pd.id, pd.doc_number, pd.kgrbs, pd.doc_date, pd.status_uid, bs.basis_type, bs.basis_number, bs.basis_date 
    FROM plan_documents pd
    INNER JOIN bases bs 
        ON bs.uid = pd.basis_uid
) pdb 
    ON pdb.id = p.document_id
GROUP BY document_id, krp, kcs, kvr, kosgu, kubp, notes
