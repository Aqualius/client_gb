-- analitic_finance source

CREATE VIEW analitic_finance AS SELECT fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, fd.doc_number, 
    STRFTIME('%d.%m.%Y', fd.doc_date) AS doc_date, 
    fd.user_comment, 
    CAST (f.amount AS Double) AS f_amount 
FROM fact_data f 
INNER JOIN fact_documents fd 
    ON fd.id = f.document_id 
GROUP BY fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, fd.doc_number,
    STRFTIME('%d.%m.%Y', fd.doc_date), fd.user_comment;


-- analitic_finance_month source

CREATE VIEW analitic_finance_month AS SELECT 
    fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes,
    CAST (SUM(amount) AS DOUBLE) AS year_sum,
    CAST (SUM(CASE WHEN fd.funded_month = 1 THEN amount ELSE 0 END) AS DOUBLE) AS m_1,
    CAST (SUM(CASE WHEN fd.funded_month = 2 THEN amount ELSE 0 END) AS DOUBLE) AS m_2,
    CAST (SUM(CASE WHEN fd.funded_month = 3 THEN amount ELSE 0 END) AS DOUBLE) AS m_3,
    CAST (SUM(CASE WHEN fd.funded_month = 4 THEN amount ELSE 0 END) AS DOUBLE) AS m_4,
    CAST (SUM(CASE WHEN fd.funded_month = 5 THEN amount ELSE 0 END) AS DOUBLE) AS m_5,
    CAST (SUM(CASE WHEN fd.funded_month = 6 THEN amount ELSE 0 END) AS DOUBLE) AS m_6,
    CAST (SUM(CASE WHEN fd.funded_month = 7 THEN amount ELSE 0 END) AS DOUBLE) AS m_7,
    CAST (SUM(CASE WHEN fd.funded_month = 8 THEN amount ELSE 0 END) AS DOUBLE) AS m_8,
    CAST (SUM(CASE WHEN fd.funded_month = 9 THEN amount ELSE 0 END) AS DOUBLE) AS m_9,
    CAST (SUM(CASE WHEN fd.funded_month = 10 THEN amount ELSE 0 END) AS DOUBLE) AS m_10,
    CAST (SUM(CASE WHEN fd.funded_month = 11 THEN amount ELSE 0 END) AS DOUBLE) AS m_11,
    CAST (SUM(CASE WHEN fd.funded_month = 12 THEN amount ELSE 0 END) AS DOUBLE) AS m_12
FROM fact_data f
INNER JOIN fact_documents fd 
    ON fd.id = f.document_id
GROUP BY fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes;


-- analitic_plan_data source

CREATE VIEW analitic_plan_data AS SELECT p.month_n, pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, Round(SUM(p.amount),2) AS sum_amount
FROM plan_data p
INNER JOIN plan_documents pd
    ON p.document_id = pd.id
GROUP BY pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, p.month_n;


-- pivot_plan_data source

CREATE VIEW pivot_plan_data AS SELECT document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,  
    CAST(SUM(CASE WHEN month_n=1  THEN amount ELSE 0 END) AS Double) AS m_1,
    CAST(SUM(CASE WHEN month_n=2  THEN amount ELSE 0 END) AS Double) AS m_2,
    CAST(SUM(CASE WHEN month_n=3  THEN amount ELSE 0 END) AS Double) AS m_3,
    CAST(SUM(CASE WHEN month_n=4  THEN amount ELSE 0 END) AS Double) AS m_4,
    CAST(SUM(CASE WHEN month_n=5  THEN amount ELSE 0 END) AS Double) AS m_5,
    CAST(SUM(CASE WHEN month_n=6  THEN amount ELSE 0 END) AS Double) AS m_6,
    CAST(SUM(CASE WHEN month_n=7  THEN amount ELSE 0 END) AS Double) AS m_7,
    CAST(SUM(CASE WHEN month_n=8  THEN amount ELSE 0 END) AS Double) AS m_8,
    CAST(SUM(CASE WHEN month_n=9  THEN amount ELSE 0 END) AS Double) AS m_9,
    CAST(SUM(CASE WHEN month_n=10 THEN amount ELSE 0 END) AS Double) AS m_10,
    CAST(SUM(CASE WHEN month_n=11 THEN amount ELSE 0 END) AS Double) AS m_11,
    CAST(SUM(CASE WHEN month_n=12 THEN amount ELSE 0 END) AS Double) AS m_12,
    CAST(SUM(amount) AS Double) AS year_sum
FROM plan_data
GROUP BY document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes;


-- pivot_plan_data_with_document source

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
    SELECT pd.id, pd.doc_number, pd.kgrbs, pd.doc_date, pd.status_uid, bs.basis_type, bs.basis_number, bs.basis_date 
    FROM plan_documents pd
    INNER JOIN bases bs 
        ON bs.uid = pd.basis_uid
) pdb 
    ON pdb.id = p.document_id
GROUP BY document_id, krp, kcs, kvr, kosgu, kubp, notes;


-- pivot_plan_data_without_document source

CREATE VIEW pivot_plan_data_without_document AS SELECT p.month_n, pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes,
    CAST(SUM(amount) AS Double) AS year_amount,  
    CAST(SUM(CASE WHEN month_n=1  THEN amount ELSE 0 END) AS Double) AS m_1,
    CAST(SUM(CASE WHEN month_n=2  THEN amount ELSE 0 END) AS Double) AS m_2,
    CAST(SUM(CASE WHEN month_n=3  THEN amount ELSE 0 END) AS Double) AS m_3,
    CAST(SUM(CASE WHEN month_n=4  THEN amount ELSE 0 END) AS Double) AS m_4,
    CAST(SUM(CASE WHEN month_n=5  THEN amount ELSE 0 END) AS Double) AS m_5,
    CAST(SUM(CASE WHEN month_n=6  THEN amount ELSE 0 END) AS Double) AS m_6,
    CAST(SUM(CASE WHEN month_n=7  THEN amount ELSE 0 END) AS Double) AS m_7,
    CAST(SUM(CASE WHEN month_n=8  THEN amount ELSE 0 END) AS Double) AS m_8,
    CAST(SUM(CASE WHEN month_n=9  THEN amount ELSE 0 END) AS Double) AS m_9,
    CAST(SUM(CASE WHEN month_n=10 THEN amount ELSE 0 END) AS Double) AS m_10,
    CAST(SUM(CASE WHEN month_n=11 THEN amount ELSE 0 END) AS Double) AS m_11,
    CAST(SUM(CASE WHEN month_n=12 THEN amount ELSE 0 END) AS Double) AS m_12
FROM plan_data
INNER JOIN plan_documents pd
    ON p.document_id = pd.id
GROUP BY pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, p.month_n;


-- remnants_incrementally source

CREATE VIEW remnants_incrementally AS SELECT kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, 
    Round(SUM(plan_amount), 2) AS "plan", 
    Round(SUM(fact_amount), 2) AS fact, 
    Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost 
FROM (
    SELECT pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, p.amount AS plan_amount, 0 AS fact_amount 
    FROM plan_data p
    INNER JOIN plan_documents pd
    WHERE month_n <= strftime('%m', date('now')) 
    UNION ALL 
    SELECT fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount 
    FROM fact_data f 
    INNER JOIN fact_documents fd 
        ON fd.id = f.document_id
    WHERE fd.funded_month <= strftime('%m', date('now'))
)
GROUP BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes;


-- remnants_incrementally_year source

CREATE VIEW remnants_incrementally_year AS SELECT month_n, kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, 
    Round(SUM("plan") OVER (PARTITION BY krp, kcs, kvr, kosgu, kubp, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW), 2) AS "plan", 
    Round(SUM(fact) OVER (PARTITION BY krp, kcs, kvr, kosgu, kubp, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW), 2) AS fact, 
    Round(SUM(ost) OVER (PARTITION BY krp, kcs, kvr, kosgu, kubp, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW), 2) AS ost 
FROM 
(
    SELECT kgrbs, month_n, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, Round(SUM(plan_amount), 2) AS "plan", Round(SUM(fact_amount), 2) AS fact, Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost 
    FROM 
    (
        SELECT pd.kgrbs, p.month_n, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, p.amount AS plan_amount, 0 AS fact_amount 
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id
        UNION ALL
        SELECT fd.kgrbs, fd.funded_month, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount 
        FROM fact_data f 
        INNER JOIN fact_documents fd 
            ON fd.id = f.document_id
    ) 
    GROUP BY krp, kcs, kvr, kosgu, kubp, notes, expenses_part, month_n
) 
ORDER BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, expenses_part, month_n;


-- remnants_year source

CREATE VIEW remnants_year AS SELECT month_n, kgrbs, krp, kcs, kvr, kosgu, kubp, notes, Round(SUM(plan_amount), 2) AS "plan", Round(SUM(fact_amount), 2) AS fact, Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost 
FROM 
(
    SELECT pd.kgrbs, month_n, krp, kcs, kvr, kosgu, kubp, notes, amount AS plan_amount, 0 AS fact_amount 
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id
    UNION ALL 
    SELECT fd.kgrbs, fd.funded_month, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.notes, 0, f.amount
    FROM fact_data f 
    INNER JOIN fact_documents fd 
        ON fd.id = f.document_id
) 
GROUP BY kgrbs, krp, kcs, kvr, kosgu, kubp, notes, month_n 
ORDER BY kgrbs, krp, kcs, kvr, kosgu, kubp, notes, month_n;


-- unpivot_expenses_editor_plan source

CREATE VIEW unpivot_expenses_editor_plan AS SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, month, amount, expenses_part, notes 
FROM 
(
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '1' AS month, m_1 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '2' AS month, m_2 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '3' AS month, m_3 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '4' AS month, m_4 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '5' AS month, m_5 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '6' AS month, m_6 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '7' AS month, m_7 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '8' AS month, m_8 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '9' AS month, m_9 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '10' AS month, m_10 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '11' AS month, m_11 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
    UNION ALL 
    SELECT id, document_id, krp, kcs, kvr, kosgu, kubp, '12' AS month, m_12 AS amount, expenses_part, notes 
    FROM expenses_editor_plan 
)
WHERE amount <> 0;