DROP VIEW pivot_plan_data_without_document;

CREATE VIEW pivot_plan_data_without_document AS SELECT pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes,
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
FROM plan_data p
INNER JOIN plan_documents pd
    ON p.document_id = pd.id
GROUP BY pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes