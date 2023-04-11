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
WHERE amount <> 0