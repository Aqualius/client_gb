DROP VIEW remnants_year;

CREATE VIEW remnants_year AS SELECT month_n, kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, Round(SUM(plan_amount), 2) AS "plan", Round(SUM(fact_amount), 2) AS fact, Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost 
FROM 
(
    SELECT pd.kgrbs, month_n, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount AS plan_amount, 0 AS fact_amount 
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id
    UNION ALL 
    SELECT fd.kgrbs, fd.funded_month, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount
    FROM fact_data f 
    INNER JOIN fact_documents fd 
        ON fd.id = f.document_id
) 
GROUP BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, month_n 
ORDER BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, month_n;
