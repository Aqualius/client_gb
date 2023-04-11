#include "sql_queries.h"

const QString Queries::kkrb_kgrbs = R"(
    SELECT kk.code, kk.title, pd.kgrbs IS NOT NULL AS checker
    FROM kkrb_kgrbs kk
    LEFT JOIN
    (
        SELECT DISTINCT kgrbs
        FROM plan_documents
    ) pd
        ON pd.kgrbs = kk.code
    WHERE NOT kk.deleted
    ORDER BY checker DESC, kk.code
)";

//////////////////////////////////
//////////// MIGRATION ///////////
//////////////////////////////////

const QString Queries::create_table_migration = R"(
    CREATE TABLE IF NOT EXISTS migrations (
        script         TEXT     NOT NULL,
        installed_on   DATETIME NOT NULL
    );
)";

const QString Queries::insert_migration = R"(
    INSERT INTO migrations (script, installed_on)
    VALUES (:script, :installed_on)
)";

const QString Queries::check_migration = R"(
    SELECT script
    FROM migrations
    WHERE script = :script
)";

//////////////////////////////////
////////////// PLAN //////////////
//////////////////////////////////

const QString Queries::kgrbs_from_doc_plan = R"(
    SELECT kgrbs FROM plan_documents pd
    WHERE id = :id
    LIMIT 1
)";

const QString Queries::kubp_plan = R"(
    SELECT rk.code, rk.title, kubp is not null AS checker
    FROM kubp rk
    LEFT JOIN
    (
        SELECT DISTINCT kubp
        FROM plan_data pd
        INNER JOIN plan_documents pd2
            ON pd2.id = document_id AND pd2.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :document_id)
    ) AS pd
        ON rk.code = kubp
    WHERE NOT rk.deleted
    ORDER BY checker DESC, rk.code
)";

const QString Queries::kkrb_krp_plan = R"(
    SELECT kk.code, kk.title, krp is not null AS checker
    FROM kkrb_krp kk
    LEFT JOIN
    (
        SELECT DISTINCT krp
        FROM plan_data pd
        INNER JOIN plan_documents pd2
            ON pd2.id = document_id AND pd2.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :document_id)
        WHERE kubp = :kubp
    ) AS pd
        ON kk.code = krp
    WHERE NOT kk.deleted
    ORDER BY checker DESC, kk.code
)";

const QString Queries::kkrb_kcs_plan = R"(
    SELECT kk.code, kk.title, kcs is not null AS checker
    FROM kkrb_kcs kk
    LEFT JOIN
    (
        SELECT DISTINCT kcs
        FROM plan_data pd
        INNER JOIN plan_documents pd2
            ON pd2.id = document_id AND pd2.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :document_id)
        WHERE kubp = :kubp AND krp = :krp
    ) AS pd
        ON kk.code = kcs
    WHERE NOT kk.deleted
    ORDER BY checker DESC, kk.code
)";

const QString Queries::kkrb_kvr_plan = R"(
    SELECT kk.code, kk.title, kvr is not null AS checker
    FROM kkrb_kvr kk
    LEFT JOIN
    (
        SELECT DISTINCT kvr
        FROM plan_data pd
        INNER JOIN plan_documents pd2
            ON pd2.id = document_id AND pd2.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :document_id)
        WHERE kubp = :kubp AND krp = :krp AND kcs = :kcs
    ) AS pd
        ON kk.code = kvr
    WHERE NOT kk.deleted
    ORDER BY checker DESC, kk.code
)";

const QString Queries::kkrb_kosgu_plan = R"(
    SELECT kk.code, kk.title, kosgu is not null AS checker
    FROM kkrb_kosgu kk
    LEFT JOIN
    (
        SELECT DISTINCT kosgu
        FROM plan_data pd
        INNER JOIN plan_documents pd2
            ON pd2.id = document_id AND pd2.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :document_id)
        WHERE kubp = :kubp AND krp = :krp AND kcs = :kcs AND kvr = :kvr
    ) AS pd
        ON kk.code = kosgu
    WHERE NOT kk.deleted
    ORDER BY checker DESC, kk.code
)";

const QString Queries::notes_plan = R"(
    SELECT DISTINCT notes
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :document_id)
    WHERE kubp = :kubp AND krp = :krp AND kcs = :kcs AND kvr = :kvr AND kosgu = :kosgu
    ORDER BY notes
)";

const QString Queries::num_from_plan_doc = R"(
    SELECT doc_number
    FROM plan_documents pd
    WHERE id = :id
    LIMIT 1
)";

const QString Queries::insert_in_exp_plan = R"(
    INSERT INTO expenses_editor_plan
        (document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12)
    SELECT document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12
    FROM pivot_plan_data
    WHERE document_id = :id
)";

const QString Queries::insert_in_exp_plan_manual = R"(
    INSERT INTO expenses_editor_plan
        (document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12)
    VALUES
        (:id, :krp, :kcs, :kvr, :kosgu, :kubp, :expenses_part, :notes, :m_1, :m_2, :m_3, :m_4, :m_5, :m_6, :m_7, :m_8, :m_9, :m_10, :m_11, :m_12)
)";

const QString Queries::delete_in_exp_plan = R"(
    DELETE FROM expenses_editor_plan
    WHERE document_id = :id
)";

const QString Queries::insert_in_plan_data = R"(
    INSERT INTO plan_data
        (document_id, month_n, amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes)
    SELECT document_id, "month", amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes
    FROM unpivot_expenses_editor_plan
    WHERE document_id = :id
)";

const QString Queries::delete_in_plan_data = R"(
    DELETE FROM plan_data
    WHERE document_id = :id
)";

const QString Queries::delete_in_plan_data_documents = R"(
    DELETE FROM plan_data
    WHERE document_id = :id
)";

const QString Queries::delete_in_plan_document = R"(
    DELETE FROM plan_documents
    WHERE id = :id
)";

const QString Queries::check_exp_plan_n_data = R"(
    SELECT CASE WHEN amount = uee.amount THEN 1 ELSE 0 END checker
    FROM plan_data pd
    LEFT JOIN unpivot_expenses_editor_plan uee
        ON document_id = uee.document_id AND krp = uee.krp AND kcs = uee.kcs AND kvr = uee.kvr AND
            kosgu = uee.kosgu AND month_n = uee.month AND notes IS uee.notes
    WHERE document_id = :id
    GROUP BY checker
    UNION ALL
    SELECT CASE WHEN amount = uee.amount THEN 1 ELSE 0 END checker
    FROM unpivot_expenses_editor_plan uee
    LEFT JOIN plan_data pd
        ON document_id = uee.document_id AND krp = uee.krp AND kcs = uee.kcs AND kvr = uee.kvr AND
            kosgu = uee.kosgu AND month_n = uee.month AND notes IS uee.notes
    WHERE uee.document_id = :id
    GROUP BY checker
)";

const QString Queries::sum_row_exp_plan = R"(
    SELECT SUM(amount) AS sum_amount
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
    WHERE %1
    GROUP BY id, document_id, krp, kcs, kvr, kosgu, kubp, notes
    ORDER BY id DESC
)";

const QString Queries::sum_col_exp_plan = R"(
    SELECT
        SUM(m_1) AS m_1,
        SUM(m_2) AS m_2,
        SUM(m_3) AS m_3,
        SUM(m_4) AS m_4,
        SUM(m_5) AS m_5,
        SUM(m_6) AS m_6,
        SUM(m_7) AS m_7,
        SUM(m_8) AS m_8,
        SUM(m_9) AS m_9,
        SUM(m_10) AS m_10,
        SUM(m_11) AS m_11,
        SUM(m_12) AS m_12
    FROM expenses_editor_plan
    WHERE %1
    LIMIT 1
)";

const QString Queries::select_plan_document = R"(
    SELECT pd.id, pd.doc_number, pd.doc_date, b.basis_type, b.basis_number, b.basis_date, pd.user_comment, s.title, s.color
    FROM plan_documents pd
    LEFT JOIN bases b
        ON b.uid = pd.basis_uid
    INNER JOIN statuses s
        ON s.uid = pd.status_uid
    WHERE kgrbs = :kgrbs AND pd.year_n = :year_n %1
    ORDER BY s.priority ASC, pd.id DESC
)";

const QString Queries::select_plan_document_with_id = R"(
    SELECT pd.doc_number, pd.doc_date, pd.basis_uid, pd.user_comment
    FROM plan_documents pd
    WHERE id = :id
    LIMIT 1
)";

const QString Queries::insert_plan_document = R"(
    INSERT INTO plan_documents
        (year_n, kgrbs, doc_number, doc_date, status_uid, basis_uid, user_comment, foreign_uuid)
    VALUES
        (:year_n, :kgrbs, :doc_number, :doc_date, :status_uid, :basis_uid, :user_comment, :foreign_uuid)
)";

const QString Queries::update_plan_document = R"(
    UPDATE plan_documents
    SET doc_number = :doc_number,
        doc_date = :doc_date,
        basis_uid = :basis_uid,
        user_comment = :user_comment
    WHERE id = :id
)";

const QString Queries::check_kkrb_exp_plan = R"(
    SELECT COUNT(id)
    FROM expenses_editor_plan
    WHERE document_id = :id AND krp = :krp AND kcs = :kcs AND kvr = :kvr AND kosgu = :kosgu AND kubp = :kubp AND
        expenses_part = :exp_part AND notes IS :notes
)";

const QString Queries::check_kosgu_with_notes = R"(
    SELECT notes_req
    FROM kkrb_kosgu
    WHERE code = :code
)";

const QString Queries::export_document_plan = R"(
    SELECT
        year_n AS year,
        kgrbs,
        CASE changes WHEN 0 THEN 'false' ELSE 'true' END AS changes,
        doc_number AS number,
        doc_date AS date,
        basis_uid AS basisUid,
        foreign_uuid AS foreignUid,
        user_comment AS comment
    FROM plan_documents
    WHERE id = :id
)";

const QString Queries::export_data_plan = R"(
    SELECT expenses_part AS expensesPart, month_n AS month, kubp, krp, kcs, kvr, kosgu, notes, amount
    FROM plan_data p
    WHERE document_id = :id
)";

const QString Queries::import_document_plan = R"(
    INSERT INTO plan_documents
        (year_n, kgrbs, changes, doc_number, doc_date, status_uid, basis_uid, user_comment, foreign_uuid)
    VALUES
        (:year, :kgrbs, :changes, :number, :date, :statusUid, :basisUid, :comment, :foreignUid)
)";

const QString Queries::import_data_plan = R"(
    INSERT INTO plan_data
        (document_id, month_n, amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes)
    VALUES
        (:document_id, :month, :amount, :krp, :kcs, :kvr, :kosgu, :kubp, :expensesPart, :notes)
)";

const QString Queries::select_status_plan_create = R"(
    SELECT uid
    FROM statuses
    WHERE document_type = 'EXPENSES_PLAN' AND status_type = 'DRAFT'
    LIMIT 1
)";

const QString Queries::select_all_bases = R"(
    SELECT DISTINCT basis_type || CASE with_changes WHEN 1 THEN ' (с изм)' ELSE '' END
            || ' №' || basis_number || ' от ' || basis_date AS basis_type,
            uid
    FROM bases b
    INNER JOIN bases_scope bs
        ON bs.basis_id = b.uid
    WHERE CAST(strftime('%Y', basis_date) AS INTEGER) = :year AND bs.kgrbs = :kgrbs
    ORDER BY b.basis_date DESC, b.basis_type DESC, b.basis_number DESC
)";

const QString Queries::select_all_bases_scope = R"(
    SELECT bs.doc_number
    FROM bases_scope bs
    INNER JOIN bases b
        ON bs.basis_id = b.uid
    WHERE CAST(strftime('%Y', b.basis_date) AS INTEGER) = :year AND bs.kgrbs = :kgrbs AND bs.basis_id = :uid;
)";

const QString Queries::select_all_bases_edit = R"(
    SELECT DISTINCT basis_type || CASE with_changes WHEN 1 THEN ' (с изм)' ELSE '' END
            || ' №' || basis_number || ' от ' || basis_date AS basis_type,
            b.uid
    FROM bases b
    INNER JOIN bases_scope bs
        ON bs.basis_id = b.uid
    WHERE CAST(strftime('%Y', b.basis_date) AS INTEGER) = :year AND bs.kgrbs = :kgrbs AND b.uid IS :uid
    UNION ALL
    SELECT *
    FROM
    (
        SELECT DISTINCT basis_type || CASE with_changes WHEN 1 THEN ' (с изм)' ELSE '' END
            || ' №' || basis_number || ' от ' || basis_date AS basis_type,
            b.uid
        FROM bases b
        INNER JOIN bases_scope bs
            ON bs.basis_id = b.uid
        WHERE CAST(strftime('%Y', b.basis_date) AS INTEGER) = :year AND bs.kgrbs = :kgrbs AND b.uid IS NOT :uid
        ORDER BY b.basis_date DESC, b.basis_type DESC, b.basis_number DESC
    )
)";

const QString Queries::check_plan = R"(
    SELECT 'План стал отрицательным' AS error, month, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
        Round(SUM(plan),2) AS plan,
        Round(SUM(edit_plan),2) AS edit_plan,
        Round(SUM(edit_plan + plan),2) AS diff
    FROM
    (
        SELECT month_n AS month, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount AS plan, 0 AS edit_plan
        FROM plan_data
        WHERE document_id <> :id
        UNION ALL
        SELECT month, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, 0, amount
        FROM unpivot_expenses_editor_plan
        WHERE document_id = :id
    ) u
    WHERE EXISTS (
        SELECT 1 FROM expenses_editor_plan
        WHERE document_id = :id AND u.krp = krp AND u.kcs = kcs AND u.kvr = kvr AND u.kosgu = kosgu AND
            u.kubp = kubp AND u.expenses_part = expenses_part AND u.notes IS notes
    )
    GROUP BY month, krp, kcs, kvr, kosgu, kubp, expenses_part, notes
    HAVING Round(SUM(edit_plan + plan),2) < 0
)";

const QString Queries::check_inc_plan = R"(
    SELECT 'Факт превышает план', *
    FROM
    (
        SELECT month_n, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
            Round(SUM("plan") OVER (PARTITION BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW), 2) AS "plan",
            Round(SUM(fact) OVER (PARTITION BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW), 2) AS fact,
            Round(SUM(ost) OVER (PARTITION BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW), 2) AS ost
        FROM
        (
            SELECT month_n, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, Round(SUM(plan_amount), 2) AS "plan", Round(SUM(fact_amount), 2) AS fact, Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost
            FROM
            (
                SELECT month_n, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount AS plan_amount, 0 AS fact_amount
                FROM plan_data
                WHERE document_id <> :id
                UNION ALL
                SELECT fd.funded_month, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount
                FROM fact_data f
                INNER JOIN fact_documents fd
                    ON fd.id = f.document_id
                UNION ALL
                SELECT month, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount, 0
                FROM unpivot_expenses_editor_plan
                WHERE document_id = :id
            ) u
            WHERE EXISTS (
                SELECT 1 FROM expenses_editor_plan
                WHERE document_id = :id AND u.krp = krp AND u.kcs = kcs AND u.kvr = kvr AND u.kosgu = kosgu AND u.kubp = kubp AND u.expenses_part = expenses_part AND u.notes IS notes
            )
            GROUP BY krp, kcs, kvr, kosgu, kubp, notes, expenses_part, month_n
        )
    )
    WHERE ost < 0
    ORDER BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes, expenses_part, month_n
)";

const QString Queries::check_id_plan = R"(
    SELECT  COUNT(DISTINCT document_id)
    FROM expenses_editor_plan
    WHERE document_id = :id
)";

const QString Queries::duplicate_plan_doc = R"(
    INSERT INTO plan_documents
        (year_n, kgrbs, changes, doc_number, doc_date, status_uid, basis_uid, user_comment, foreign_uuid)
    SELECT
        year_n, kgrbs, changes, doc_number, doc_date, s.uid, basis_uid, user_comment, :foreign_uuid
    FROM plan_documents
    INNER JOIN statuses s
        ON s.document_type = 'EXPENSES_PLAN' AND s.status_type = 'DRAFT'
    WHERE id = :id
)";

const QString Queries::duplicate_plan_data = R"(
    INSERT INTO plan_data
        (document_id, month_n, amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes)
    SELECT
        :document_id, month_n, amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes
    FROM plan_data
    WHERE document_id = :id
)";

const QString Queries::check_num_plan_doc = R"(
    SELECT DISTINCT doc_number
    FROM plan_documents
    WHERE doc_number = :num
)";

const QString Queries::solution_plan_without_kosgu = R"(
    SELECT pd.kgrbs, p.krp, p.kcs, p.kvr, SUM(p.amount) AS amount
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id
    WHERE pd.id = :id
    GROUP BY pd.kgrbs, p.krp, p.kcs, p.kvr
    ORDER BY pd.kgrbs, p.krp, p.kcs, p.kvr
)";

const QString Queries::solution_plan_with_kosgu = R"(
    SELECT pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, SUM(p.amount) AS amount
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id
    WHERE pd.id = :id
    GROUP BY pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu
    ORDER BY pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu
)";

const QString Queries::insert_minus_kcs = R"(
    INSERT INTO plan_data
        (document_id, month_n,krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount)
    SELECT :id, month_n, krp, kcs, kvr, kosgu, :old_kubp, expenses_part, notes, -SUM(amount) AS amount
    FROM plan_data
    WHERE kcs = :kcs AND kubp = :old_kubp
    GROUP BY month_n, krp, kcs, kvr, kosgu, expenses_part, notes
)";

const QString Queries::insert_plus_kcs = R"(
    INSERT INTO plan_data
        (document_id, month_n,krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount)
    SELECT :id, month_n, krp, kcs, kvr, kosgu, :new_kubp, expenses_part, notes, SUM(amount) AS amount
    FROM plan_data
    WHERE kcs = :kcs AND kubp = :old_kubp
    GROUP BY month_n, krp, kcs, kvr, kosgu, expenses_part, notes
)";

const QString Queries::insert_minus_krp_kcs = R"(
    INSERT INTO plan_data
        (document_id, month_n,krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount)
    SELECT :id, month_n, krp, kcs, kvr, kosgu, :old_kubp, expenses_part, notes, -SUM(amount) AS amount
    FROM plan_data
    WHERE krp = :krp AND kcs = :kcs AND kubp = :old_kubp
    GROUP BY month_n, krp, kcs, kvr, kosgu, expenses_part, notes
)";

const QString Queries::insert_plus_krp_kcs = R"(
    INSERT INTO plan_data
        (document_id, month_n,krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount)
    SELECT :id, month_n, krp, kcs, kvr, kosgu, :new_kubp, expenses_part, notes, SUM(amount) AS amount
    FROM plan_data
    WHERE krp = :krp AND kcs = :kcs AND kubp = :old_kubp
    GROUP BY month_n, krp, kcs, kvr, kosgu, expenses_part, notes
)";

const QString Queries::check_plan_by_foreign_uid = R"(
    SELECT DISTINCT id
    FROM plan_documents
    WHERE foreign_uuid = :uid
)";

const QString Queries::delete_plan_doc_by_foreign_uid = R"(
    DELETE FROM plan_documents
    WHERE foreign_uuid = :uid
)";

const QString Queries::delete_plan_data_by_foreign_uid = R"(
    DELETE FROM plan_data
    WHERE document_id IN (SELECT id FROM plan_documents WHERE foreign_uuid = :uid)
)";

const QString Queries::check_duplicate_exp_plan = R"(
    SELECT krp, kcs, kvr, kosgu, kubp, expenses_part, notes, COUNT(*) c
    FROM expenses_editor_plan
    WHERE document_id = :id
    GROUP BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes
    HAVING c > 1
)";

const QString Queries::insert_minus_ost_in_exp_plan = R"(
    INSERT INTO expenses_editor_plan
        (document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12)
    SELECT :id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
        -CAST(SUM(CASE WHEN month_n=1  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_1,
        -CAST(SUM(CASE WHEN month_n=2  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_2,
        -CAST(SUM(CASE WHEN month_n=3  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_3,
        -CAST(SUM(CASE WHEN month_n=4  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_4,
        -CAST(SUM(CASE WHEN month_n=5  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_5,
        -CAST(SUM(CASE WHEN month_n=6  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_6,
        -CAST(SUM(CASE WHEN month_n=7  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_7,
        -CAST(SUM(CASE WHEN month_n=8  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_8,
        -CAST(SUM(CASE WHEN month_n=9  THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_9,
        -CAST(SUM(CASE WHEN month_n=10 THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_10,
        -CAST(SUM(CASE WHEN month_n=11 THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_11,
        -CAST(SUM(CASE WHEN month_n=12 THEN plan_amount - fact_amount ELSE 0 END) AS Double) AS m_12
    FROM
    (
        SELECT month_n, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount AS plan_amount, 0 AS fact_amount
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id
        WHERE pd.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :id) %1
        UNION ALL
        SELECT fd.funded_month, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount
        FROM fact_data f
        INNER JOIN fact_documents fd
            ON fd.id = f.document_id
        WHERE fd.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :id) %1
    )
    GROUP BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes
)";

const QString Queries::insert_minus_in_exp_plan = R"(
    INSERT INTO expenses_editor_plan
        (document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12)
    SELECT :id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
        -CAST(SUM(CASE WHEN month_n=1  THEN p.amount ELSE 0 END) AS Double) AS m_1,
        -CAST(SUM(CASE WHEN month_n=2  THEN p.amount ELSE 0 END) AS Double) AS m_2,
        -CAST(SUM(CASE WHEN month_n=3  THEN p.amount ELSE 0 END) AS Double) AS m_3,
        -CAST(SUM(CASE WHEN month_n=4  THEN p.amount ELSE 0 END) AS Double) AS m_4,
        -CAST(SUM(CASE WHEN month_n=5  THEN p.amount ELSE 0 END) AS Double) AS m_5,
        -CAST(SUM(CASE WHEN month_n=6  THEN p.amount ELSE 0 END) AS Double) AS m_6,
        -CAST(SUM(CASE WHEN month_n=7  THEN p.amount ELSE 0 END) AS Double) AS m_7,
        -CAST(SUM(CASE WHEN month_n=8  THEN p.amount ELSE 0 END) AS Double) AS m_8,
        -CAST(SUM(CASE WHEN month_n=9  THEN p.amount ELSE 0 END) AS Double) AS m_9,
        -CAST(SUM(CASE WHEN month_n=10 THEN p.amount ELSE 0 END) AS Double) AS m_10,
        -CAST(SUM(CASE WHEN month_n=11 THEN p.amount ELSE 0 END) AS Double) AS m_11,
        -CAST(SUM(CASE WHEN month_n=12 THEN p.amount ELSE 0 END) AS Double) AS m_12
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id
    WHERE pd.kgrbs IN (SELECT kgrbs FROM plan_documents WHERE id = :id) %1
    GROUP BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes
)";

const QString Queries::insert_merge_in_exp_plan = R"(
    INSERT INTO expenses_editor_plan
        (document_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
         m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8, m_9, m_10, m_11, m_12)
    SELECT :new_id, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
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
    WHERE document_id = :old_id
    GROUP BY krp, kcs, kvr, kosgu, kubp, expenses_part, notes
)";

//////////////////////////////////
////////////// FACT //////////////
//////////////////////////////////

const QString Queries::kubp_fact = R"(
    SELECT rk.code, rk.title
    FROM kubp rk
    LEFT JOIN
    (
        SELECT DISTINCT kubp
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :document_id)
        WHERE month_n <= :month_n
    ) AS fd
        ON rk.code = fd.kubp
    WHERE NOT rk.deleted AND fd.kubp IS NOT NULL
    ORDER BY rk.code
)";

const QString Queries::kkrb_krp_fact = R"(
    SELECT kk.code, kk.title
    FROM kkrb_krp kk
    LEFT JOIN
    (
        SELECT DISTINCT krp
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :document_id)
        WHERE kubp = :kubp AND month_n <= :month_n
    ) AS fd
        ON kk.code = fd.krp
    WHERE NOT kk.deleted AND fd.krp IS NOT NULL
    ORDER BY kk.code
)";

const QString Queries::kkrb_kcs_fact = R"(
    SELECT kk.code, kk.title
    FROM kkrb_kcs kk
    LEFT JOIN
    (
        SELECT DISTINCT kcs
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :document_id)
        WHERE kubp = :kubp AND krp = :krp AND month_n <= :month_n
    ) AS fd
        ON kk.code = fd.kcs
    WHERE NOT kk.deleted AND fd.kcs IS NOT NULL
    ORDER BY kk.code
)";

const QString Queries::kkrb_kvr_fact = R"(
    SELECT kk.code, kk.title
    FROM kkrb_kvr kk
    LEFT JOIN
    (
        SELECT DISTINCT kvr
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :document_id)
        WHERE kubp = :kubp AND krp = :krp AND kcs = :kcs AND month_n <= :month_n
    ) AS fd
        ON kk.code = fd.kvr
    WHERE NOT kk.deleted AND fd.kvr IS NOT NULL
    ORDER BY kk.code
)";

const QString Queries::kkrb_kosgu_fact = R"(
    SELECT kk.code, kk.title
    FROM kkrb_kosgu kk
    LEFT JOIN
    (
        SELECT DISTINCT kosgu
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :document_id)
        WHERE kubp = :kubp AND krp = :krp AND kcs = :kcs AND kvr = :kvr AND month_n <= :month_n
    ) AS fd
        ON kk.code = fd.kosgu
    WHERE NOT kk.deleted AND fd.kosgu IS NOT NULL
    ORDER BY kk.code
)";

const QString Queries::notes_fact = R"(
    SELECT DISTINCT notes
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :document_id)
    WHERE kubp = :kubp AND krp = :krp AND kcs = :kcs AND kvr = :kvr AND kosgu = :kosgu AND month_n <= :month_n
    ORDER BY notes
)";

const QString Queries::exp_part_fact = R"(
    SELECT DISTINCT
        CASE expenses_part
            WHEN 'CURRENT' THEN 'Текущие'
            WHEN 'CAPITAL' THEN 'Капитальные'
            WHEN 'CREDIT_ISSUE' THEN 'Предоставление кредитов'
            WHEN 'CREDIT_REFUND' THEN 'Возврат кредитов'
            WHEN 'RESERVE_FUND' THEN 'Резервный фонд'
            WHEN 'ROAD_FUND' THEN 'Дорожный фонд'
            WHEN 'COAL_INDUSTRY_FUND' THEN 'Угольный фонд'
            WHEN 'INDUSTRY_FUND' THEN 'Промышленный фонд'
        END
    FROM plan_data p
    INNER JOIN plan_documents pd
        ON pd.id = p.document_id AND kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :document_id)
    WHERE kubp = :kubp AND krp = :krp AND kcs = :kcs AND kvr = :kvr AND kosgu = :kosgu AND notes IS :notes AND month_n <= :month_n
    ORDER BY expenses_part
)";

const QString Queries::direction_fact = R"(
    SELECT DISTINCT title
    FROM direction_expenditure
    ORDER BY title
)";

const QString Queries::num_from_fact_doc = R"(
    SELECT doc_number
    FROM fact_documents
    WHERE id = :id
    LIMIT 1
)";

const QString Queries::insert_in_exp_fact = R"(
    INSERT INTO expenses_editor_fact
        (document_id, krp, kcs, kvr, kosgu, kubp, direction, notes, expenses_part, amount)
    SELECT document_id, krp, kcs, kvr, kosgu, kubp, direction, notes, expenses_part, amount
    FROM fact_data
    WHERE document_id = :id
)";

const QString Queries::insert_in_exp_fact_manual = R"(
    INSERT INTO expenses_editor_fact
        (document_id, krp, kcs, kvr, kosgu, kubp, direction, notes, expenses_part, amount)
    VALUES
        (:id, :krp, :kcs, :kvr, :kosgu, :kubp, :direction, :notes, :expenses_part, :amount)
)";

const QString Queries::delete_in_exp_fact = R"(
    DELETE FROM expenses_editor_fact
    WHERE document_id = :id
)";

const QString Queries::insert_in_fact_data = R"(
    INSERT INTO fact_data
        (document_id, krp, kcs, kvr, kosgu, kubp, notes, direction, expenses_part, amount)
    SELECT document_id, krp, kcs, kvr, kosgu, kubp, notes, direction, expenses_part, amount
    FROM expenses_editor_fact
    WHERE document_id = :id
)";

const QString Queries::delete_in_fact_data = R"(
    DELETE FROM fact_data
    WHERE document_id = :id
)";

const QString Queries::delete_in_fact_document = R"(
    DELETE FROM fact_documents
    WHERE id = :id
)";

const QString Queries::check_exp_fact_n_data = R"(

)";

const QString Queries::balance_plan_n_fact = R"(
    SELECT Round(SUM(pf.p_sum) - IFNULL(SUM(pf.f_sum), 0) - eef.amount, 2) AS ost, '',
        Round(SUM(pf.p_sum) - IFNULL(SUM(pf.f_sum), 0) - eef.amount, 2) >= -0 AS checker
    FROM expenses_editor_fact eef
    LEFT JOIN (
        SELECT kubp, krp, kcs, kvr, kosgu, expenses_part, notes, amount AS p_sum, 0 AS f_sum
        FROM plan_data p
        WHERE month_n <= :month_n
        UNION ALL
        SELECT kubp, krp, kcs, kvr, kosgu, expenses_part, notes, 0, amount
        FROM  fact_data f
        WHERE f.document_id <> :id
    ) pf
        ON pf.kubp = eef.kubp AND pf.krp = eef.krp AND pf.kcs = eef.kcs AND pf.kvr = eef.kvr AND pf.kosgu = eef.kosgu  AND pf.expenses_part = eef.expenses_part AND
            pf.notes IS eef.notes
    WHERE %1
    GROUP BY eef.kubp, eef.krp, eef.kcs, eef.kvr, eef.kosgu, eef.expenses_part, eef.notes
    ORDER BY eef.id DESC
)";

const QString Queries::balance_fact_refund = R"(
    SELECT Round(SUM(f.amount) + eef.amount, 2) AS ost, '',
        Round(SUM(f.amount) + eef.amount, 2)  >= -0 AS checker
    FROM expenses_editor_fact eef
    LEFT JOIN fact_data f
        ON f.kubp = eef.kubp AND f.krp = eef.krp AND f.kcs = eef.kcs AND f.kvr = eef.kvr AND f.kosgu = eef.kosgu  AND f.expenses_part = eef.expenses_part AND
            f.notes IS eef.notes AND f.document_id <> :id
    WHERE %1
    GROUP BY eef.kubp, eef.krp, eef.kcs, eef.kvr, eef.kosgu, eef.expenses_part, eef.notes
    ORDER BY eef.id DESC
)";

const QString Queries::sum_col_exp_fact = R"(
    SELECT SUM(amount) AS sum_amount
    FROM expenses_editor_fact
    WHERE %1
    LIMIT 1
)";

const QString Queries::select_fact_document = R"(
    SELECT fd.id, fd.payment_type, fd.doc_number, fd.doc_date, fd.funded_at, fd.user_comment, s.title, s.color, SUM(f.amount) AS amount
    FROM fact_documents fd
    INNER JOIN statuses s
        ON s.uid = fd.status_uid
    LEFT JOIN fact_data f
        ON f.document_id = fd.id
    WHERE fd.kgrbs = :kgrbs AND fd.year_n = :year_n %1
    GROUP BY fd.id, fd.payment_type, fd.doc_number, fd.doc_date, fd.user_comment, s.title, s.color
    ORDER BY s.priority ASC, fd.id DESC
)";

const QString Queries::select_fact_document_with_id = R"(
    SELECT doc_number, doc_date, user_comment, payment_type
    FROM fact_documents
    WHERE id = :id
    LIMIT 1
)";

const QString Queries::insert_fact_document = R"(
    INSERT INTO fact_documents
        (year_n, kgrbs, doc_number, doc_date, status_uid, payment_type, user_comment, foreign_uuid)
    VALUES
        (:year_n, :kgrbs, :doc_number, :doc_date, :status_uid, :payment_type, :user_comment, :foreign_uuid)
)";

const QString Queries::update_fact_document = R"(
    UPDATE fact_documents
    SET doc_number = :doc_number,
        doc_date = :doc_date,
        user_comment = :user_comment,
        payment_type = :payment_type
    WHERE id = :id
)";

const QString Queries::check_kkrb_exp_fact = R"(
    SELECT COUNT(id)
    FROM expenses_editor_fact
    WHERE document_id = :id AND krp = :krp AND kcs = :kcs AND kvr = :kvr AND kosgu = :kosgu AND kubp = :kubp AND
        expenses_part = :exp_part AND notes IS :notes
)";

const QString Queries::export_document_fact = R"(
    SELECT
        year_n AS year, kgrbs, doc_number AS number, doc_date AS date, payment_type AS paymentType,
        foreign_uuid AS foreignUid, user_comment AS comment
    FROM fact_documents
    WHERE id = :id
)";

const QString Queries::export_data_fact = R"(
    SELECT expenses_part AS expensesPart, kubp, krp, kcs, kvr, kosgu, notes, amount
    FROM fact_data p
    WHERE document_id = :id
)";

const QString Queries::import_document_fact = R"(
    INSERT INTO fact_documents
        (year_n, kgrbs, doc_number, doc_date, funded_at, payment_type, status_uid, foreign_uuid, user_comment)
    VALUES
        (:year, :kgrbs, :number, :date, :fundedAt, :paymentType, :statusUid, :foreignUid, :comment)
)";

const QString Queries::import_data_fact = R"(
    INSERT INTO fact_data
        (document_id, amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes)
    VALUES
        (:document_id, :amount, :krp, :kcs, :kvr, :kosgu, :kubp, :expensesPart, :notes)
)";

const QString Queries::select_status_fact_create = R"(
    SELECT uid
    FROM statuses
    WHERE document_type = 'EXPENSES_FACT' AND status_type = 'DRAFT'
    LIMIT 1
)";

const QString Queries::duplicate_fact_doc = R"(
    INSERT INTO fact_documents
        (year_n, kgrbs, doc_number, doc_date, payment_type, status_uid, foreign_uuid, user_comment)
    SELECT
        year_n, kgrbs, doc_number, doc_date, payment_type, s.uid, :foreign_uuid, user_comment
    FROM fact_documents fd
    INNER JOIN statuses s
        ON s.document_type = 'EXPENSES_FACT' AND s.status_type = 'DRAFT'
    WHERE id = :id
)";

const QString Queries::duplicate_fact_data = R"(
    INSERT INTO fact_data
        (document_id, amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes)
    SELECT
        :document_id, amount, krp, kcs, kvr, kosgu, kubp, expenses_part, notes
    FROM fact_data
    WHERE document_id = :id
)";

const QString Queries::check_id_fact = R"(
    SELECT DISTINCT document_id
    FROM expenses_editor_fact
    WHERE document_id = :id
)";

const QString Queries::check_num_fact_doc = R"(
    SELECT DISTINCT doc_number
    FROM fact_documents
    WHERE doc_number = :num
)";

const QString Queries::check_fact_by_foreign_uid = R"(
    SELECT DISTINCT id
    FROM fact_documents
    WHERE foreign_uuid = :uid
)";

const QString Queries::delete_fact_doc_by_foreign_uid = R"(
    DELETE FROM fact_documents
    WHERE foreign_uuid = :uid
)";

const QString Queries::delete_fact_data_by_foreign_uid = R"(
    DELETE FROM fact_data
    WHERE document_id IN (SELECT id FROM fact_documents WHERE foreign_uuid = :uid)
)";

const QString Queries::check_duplicate_exp_fact = R"(
    SELECT krp, kcs, kvr, kosgu, kubp, notes, direction, expenses_part, COUNT(*) c
    FROM expenses_editor_fact
    WHERE document_id = :id
    GROUP BY krp, kcs, kvr, kosgu, kubp, notes, direction, expenses_part
    HAVING c > 1
)";

const QString Queries::get_type_doc_fact = R"(
    SELECT payment_type
    FROM fact_documents
    WHERE id = :id
)";

const QString Queries::insert_merge_in_exp_fact = R"(
    INSERT INTO expenses_editor_fact
        (document_id, krp, kcs, kvr, kosgu, kubp, direction, notes, expenses_part, amount)
    SELECT :new_id, krp, kcs, kvr, kosgu, kubp, direction, notes, expenses_part, amount
    FROM fact_data
    WHERE document_id = :old_id
)";

const QString Queries::insert_minus_in_exp_fact = R"(
    INSERT INTO expenses_editor_fact
        (document_id, krp, kcs, kvr, kosgu, kubp, notes, direction, expenses_part, amount)
    SELECT :id, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.notes, f.direction, f.expenses_part, -SUM(f.amount)
    FROM fact_data f
    INNER JOIN fact_documents fd
        ON fd.id = f.document_id
    WHERE fd.kgrbs IN (SELECT kgrbs FROM fact_documents WHERE id = :id) %1
    GROUP BY f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes
)";

//////////////////////////////////
//////////// ANALITICS ///////////
//////////////////////////////////

const QString Queries::sum_analitic_plan_data = R"(
    SELECT SUM(sum_amount)
    FROM analitic_plan_data
)";

const QString Queries::sum_analitic_plan_data_pivot = R"(
    SELECT SUM(year_amount) AS sum_year,
        SUM(m_1) AS sum_m_1,
        SUM(m_2) AS sum_m_2,
        SUM(m_3) AS sum_m_3,
        SUM(m_4) AS sum_m_4,
        SUM(m_5) AS sum_m_5,
        SUM(m_6) AS sum_m_6,
        SUM(m_7) AS sum_m_7,
        SUM(m_8) AS sum_m_8,
        SUM(m_9) AS sum_m_9,
        SUM(m_10) AS sum_m_10,
        SUM(m_11) AS sum_m_11,
        SUM(m_12) AS sum_m_12
    FROM pivot_plan_data_without_document
)";

const QString Queries::sum_analitic_plan_data_document_pivot = R"(
    SELECT SUM(year_amount) AS sum_year,
        SUM(m_1) AS sum_m_1,
        SUM(m_2) AS sum_m_2,
        SUM(m_3) AS sum_m_3,
        SUM(m_4) AS sum_m_4,
        SUM(m_5) AS sum_m_5,
        SUM(m_6) AS sum_m_6,
        SUM(m_7) AS sum_m_7,
        SUM(m_8) AS sum_m_8,
        SUM(m_9) AS sum_m_9,
        SUM(m_10) AS sum_m_10,
        SUM(m_11) AS sum_m_11,
        SUM(m_12) AS sum_m_12
    FROM pivot_plan_data_with_document
)";

const QString Queries::sum_analitic_finance_year = R"(
    SELECT SUM(f_amount) AS sum_amount
    FROM analitic_finance
)";

const QString Queries::sum_analitic_finance_month = R"(
    SELECT
        SUM(year_sum) AS year_sum,
        SUM(m_1) AS m_1,
        SUM(m_2) AS m_2,
        SUM(m_3) AS m_3,
        SUM(m_4) AS m_4,
        SUM(m_5) AS m_5,
        SUM(m_6) AS m_6,
        SUM(m_7) AS m_7,
        SUM(m_8) AS m_8,
        SUM(m_9) AS m_9,
        SUM(m_10) AS m_10,
        SUM(m_11) AS m_11,
        SUM(m_12) AS m_12
    FROM analitic_finance_month
)";

const QString Queries::sum_remnants_year = R"(
    SELECT SUM(plan) AS plan,
        SUM(fact) AS fact,
        SUM(ost) AS ost
    FROM remnants_year
)";

const QString Queries::sum_remnants_incrementally = R"(
    SELECT SUM(plan) AS plan,
        SUM(fact) AS fact,
        SUM(ost) AS ost
    FROM remnants_incrementally
)";

const QString Queries::sum_remnants_incrementally_year = R"(
    SELECT SUM(plan) AS plan,
        SUM(fact) AS fact,
        SUM(ost) AS ost
    FROM remnants_incrementally_year
)";

const QString Queries::sum_remnants_incrementally_period = R"(
    SELECT SUM(plan) AS plan,
        SUM(fact) AS fact,
        SUM(ost) AS ost
    FROM remnants_incrementally_period
)";

const QString Queries::remnants_incrementally = R"(
    SELECT
        SUM(plan) OVER (
          PARTITION BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
        ) AS plan,
        SUM(fact) OVER (
          PARTITION BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
        ) AS fact,
        SUM(ost) OVER (
          PARTITION BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes ORDER BY month_n RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
        ) AS ost
    FROM (
        SELECT month_n, kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, Round(SUM(plan_amount), 2) AS plan, Round(SUM(fact_amount), 2) AS fact, Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost
        FROM (
            SELECT month_n, kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, amount AS plan_amount, 0 AS fact_amount
            FROM plan_data p
            INNER JOIN plan_documents pd
                ON pd.id = p.document_id
            %1
            UNION ALL
            SELECT fd.funded_month AS month_n, fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0 AS plan, f.amount AS fact
            FROM fact_data f
            INNER JOIN fact_documents fd
                ON fd.id = f.document_id
            %1
        )
        GROUP BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, month_n
        ORDER BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, month_n
    )
    ORDER BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes, month_n
)";

const QString Queries::remnants_incrementally_current = R"(
    SELECT kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
        Round(SUM(plan_amount), 2) AS "plan",
        Round(SUM(fact_amount), 2) AS fact,
        Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost
    FROM (
        SELECT pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, p.amount AS plan_amount, 0 AS fact_amount
        FROM plan_data p
        INNER JOIN plan_documents pd
        WHERE month_n >= :month_start AND month_n <= :month_end
        UNION ALL
        SELECT fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount
        FROM fact_data f
        INNER JOIN fact_documents fd
            ON fd.id = f.document_id
        WHERE fd.funded_month >= :month_start AND fd.funded_month <= :month_end
    )
    GROUP BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes
)";

const QString Queries::drop_remnants_incrementally_period = R"(
    DROP VIEW IF EXISTS remnants_incrementally_period
)";

const QString Queries::create_remnants_incrementally_period = R"(
    CREATE TEMPORARY VIEW IF NOT EXISTS remnants_incrementally_period AS
    SELECT kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes,
        Round(SUM(plan_amount), 2) AS "plan",
        Round(SUM(fact_amount), 2) AS fact,
        Round(SUM(plan_amount) - SUM(fact_amount), 2) AS ost
    FROM (
        SELECT pd.kgrbs, p.krp, p.kcs, p.kvr, p.kosgu, p.kubp, p.expenses_part, p.notes, p.amount AS plan_amount, 0 AS fact_amount
        FROM plan_data p
        INNER JOIN plan_documents pd
            ON pd.id = p.document_id
        WHERE month_n >= %1 AND month_n <= %2
        UNION ALL
        SELECT fd.kgrbs, f.krp, f.kcs, f.kvr, f.kosgu, f.kubp, f.expenses_part, f.notes, 0, f.amount
        FROM fact_data f
        INNER JOIN fact_documents fd
            ON fd.id = f.document_id
        WHERE fd.doc_date >= '%3' AND fd.doc_date <= '%4'
    )
    GROUP BY kgrbs, krp, kcs, kvr, kosgu, kubp, expenses_part, notes
)";

//////////////////////////////////
///////////// UPDATE /////////////
//////////////////////////////////

const QString Queries::delete_statuses = R"(
    DELETE FROM statuses
)";

const QString Queries::insert_statuses = R"(
    INSERT INTO statuses (uid, document_type, title, status_type, priority, color)
    VALUES (:uid, :documentType, :title, :statusType, :priority, :color)
)";

const QString Queries::delete_kubp = R"(
    DELETE FROM kubp
)";

const QString Queries::insert_kubp = R"(
    INSERT INTO kubp (code, kgrbs, title, deleted)
    VALUES (:code, :kgrbs, :title, :deleted)
)";

const QString Queries::delete_kgrbs = R"(
    DELETE FROM kkrb_kgrbs
)";

const QString Queries::insert_kgrbs = R"(
    INSERT INTO kkrb_kgrbs (code, title, deleted)
    VALUES (:code, :title, :deleted)
)";

const QString Queries::delete_krp = R"(
    DELETE FROM kkrb_krp
)";

const QString Queries::insert_krp = R"(
    INSERT INTO kkrb_krp (code, title, deleted)
    VALUES (:code, :title, :deleted)
)";

const QString Queries::delete_kcs = R"(
    DELETE FROM kkrb_kcs
)";

const QString Queries::insert_kcs = R"(
    INSERT INTO kkrb_kcs (code, title, deleted)
    VALUES (:code, :title, :deleted)
)";

const QString Queries::delete_kvr = R"(
    DELETE FROM kkrb_kvr
)";

const QString Queries::insert_kvr = R"(
    INSERT INTO kkrb_kvr (code, title, deleted)
    VALUES (:code, :title, :deleted)
)";

const QString Queries::delete_kosgu = R"(
    DELETE FROM kkrb_kosgu
)";

const QString Queries::insert_kosgu = R"(
    INSERT INTO kkrb_kosgu (code, title, deleted, notes_req)
    VALUES (:code, :title, :deleted, :notes_req)
)";

const QString Queries::delete_bases = R"(
    DELETE FROM bases
)";

const QString Queries::insert_bases = R"(
    INSERT INTO bases (uid, basis_type, basis_number, basis_date, with_changes)
    VALUES (:uid, :basis_type, :basis_number, :basis_date, :with_changes)
)";

const QString Queries::delete_bases_scope = R"(
    DELETE FROM bases_scope
)";

const QString Queries::insert_bases_scope = R"(
    INSERT INTO bases_scope (basis_id, doc_number, kgrbs)
    VALUES (:uid, :docNumber, :kgrbs)
)";

const QString Queries::delete_plan_doc = R"(
    DELETE FROM plan_documents
    WHERE status_uid NOT IN (
        SELECT uid
        FROM statuses
        WHERE status_type = 'DRAFT' AND document_type = 'EXPENSES_PLAN'
    )
)";

const QString Queries::delete_plan_data = R"(
    DELETE FROM plan_data
    WHERE document_id NOT IN (
        SELECT id
        FROM plan_documents
    )
)";

const QString Queries::insert_plan_doc = R"(
    INSERT INTO plan_documents (year_n, kgrbs, changes, doc_number, doc_date, status_uid, basis_uid, user_comment, foreign_uuid)
    VALUES (:year_n, :kgrbs, :changes, :doc_number, :doc_date, :status_uid, :basis_uid, :user_comment, :foreign_uuid)
)";

const QString Queries::delete_fact_doc = R"(
    DELETE FROM fact_documents
    WHERE status_uid NOT IN (
        SELECT uid
        FROM statuses
        WHERE status_type = 'DRAFT' AND document_type = 'EXPENSES_FACT'
    )
)";
const QString Queries::delete_fact_data = R"(
    DELETE FROM fact_data
    WHERE document_id NOT IN (
        SELECT id
        FROM fact_documents
    )
)";
