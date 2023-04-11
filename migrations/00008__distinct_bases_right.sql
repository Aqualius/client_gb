
-- unpivot_expenses_editor_plan source
DROP VIEW unpivot_expenses_editor_plan;

CREATE VIEW unpivot_expenses_editor_plan AS SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, month, amount, expenses_part, notes 
FROM 
(
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 1 AS month, m_1 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 2 AS month, m_2 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 3 AS month, m_3 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 4 AS month, m_4 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 5 AS month, m_5 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 6 AS month, m_6 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 7 AS month, m_7 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 8 AS month, m_8 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 9 AS month, m_9 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 10 AS month, m_10 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 11 AS month, m_11 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, 12 AS month, m_12 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
)
WHERE amount <> 0;

DROP VIEW pivot_plan_data_with_document;

CREATE VIEW pivot_plan_data_with_document AS SELECT 
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
GROUP BY document_id, krp, kcs, kvr, kosgu, kubp, notes;