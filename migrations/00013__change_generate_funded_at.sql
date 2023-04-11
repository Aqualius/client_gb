
CREATE TEMPORARY TABLE fact_documents_backup (
    id           INTEGER        NOT NULL,
    year_n       INT2           NOT NULL,
    kgrbs        VARCHAR (3),
    doc_number   VARCHAR (32),
    doc_date     TIMESTAMP (13) NOT NULL,
    status_uid   VARCHAR        NOT NULL
                                REFERENCES statuses (uid) ON DELETE CASCADE
                                                          ON UPDATE CASCADE
                                                          MATCH SIMPLE,
    payment_type TEXT           NOT NULL,
    user_comment VARCHAR (255),
    foreign_uuid VARCHAR
);

INSERT INTO fact_documents_backup SELECT id, year_n, kgrbs, doc_number, doc_date, status_uid, payment_type, user_comment, foreign_uuid FROM fact_documents;

DROP TABLE fact_documents;

CREATE TABLE fact_documents (
    id           INTEGER        NOT NULL,
    year_n       INT2           NOT NULL,
    kgrbs        VARCHAR (3),
    doc_number   VARCHAR (32),
    doc_date     TIMESTAMP (13) NOT NULL,
    status_uid   VARCHAR        NOT NULL
                                REFERENCES statuses (uid) ON DELETE CASCADE
                                                          ON UPDATE CASCADE
                                                          MATCH SIMPLE,
    payment_type TEXT           NOT NULL,
    user_comment VARCHAR (255),
    foreign_uuid VARCHAR,
    funded_month INTEGER        AS (CASE WHEN payment_type = 'REFUND' THEN 1 ELSE strftime('%m', doc_date) END),
    CONSTRAINT FACT_DOCUMENTS_PK PRIMARY KEY (
        id
    ),
    CONSTRAINT FK_fact_documents_kkrb_kgrbs FOREIGN KEY (
        kgrbs
    )
    REFERENCES kkrb_kgrbs (code) 
);

INSERT INTO fact_documents SELECT id, year_n, kgrbs, doc_number, doc_date, status_uid, payment_type, user_comment, foreign_uuid FROM fact_documents_backup;

DROP TABLE fact_documents_backup;
