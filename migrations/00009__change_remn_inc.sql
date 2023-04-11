DROP VIEW remnants_incrementally;

CREATE VIEW remnants_incrementally AS SELECT kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, 
    Round(SUM(plan_amount), 2) AS "plan", 
    Round(SUM(fact_amount), 2) AS fact, 
    Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost 
FROM (
    SELECT pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, p.amount AS plan_amount, 0 AS fact_amount 
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id
    WHERE month_n <= strftime('%m', date('now')) 
    UNION ALL 
    SELECT fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount 
    FROM fact_data f 
    INNER JOIN fact_documents fd 
        ON fd.id = f.document_id
    WHERE fd.funded_month <= strftime('%m', date('now'))
)
GROUP BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes;
