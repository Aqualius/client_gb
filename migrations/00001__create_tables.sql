-- bases definition

CREATE TABLE bases (uid VARCHAR NOT NULL PRIMARY KEY, year_n INTEGER NOT NULL, basis_type VARCHAR, basis_number VARCHAR, basis_date VARCHAR NOT NULL, with_changes BOOLEAN NOT NULL DEFAULT (false)) WITHOUT ROWID;

-- document_status definition

CREATE TABLE document_status (id INTEGER PRIMARY KEY, document_type VARCHAR, title VARCHAR, uid VARCHAR);

-- kkrb_kgrbs definition

CREATE TABLE kkrb_kgrbs (code varchar (3) PRIMARY KEY, title text, deleted BOOLEAN NOT NULL);

-- kkrb_krp definition

CREATE TABLE kkrb_krp (code varchar (4) PRIMARY KEY, title text, deleted BOOLEAN NOT NULL);

-- kkrb_kcs definition

CREATE TABLE kkrb_kcs (code varchar (7) PRIMARY KEY, title text, deleted BOOLEAN NOT NULL);

-- kkrb_kvr definition

CREATE TABLE kkrb_kvr (code varchar (3) PRIMARY KEY, title text, deleted BOOLEAN NOT NULL);

-- kkrb_kosgu definition

CREATE TABLE kkrb_kosgu (code varchar (3) PRIMARY KEY, title text, deleted BOOLEAN NOT NULL);

-- kubp definition

CREATE TABLE kubp (code varchar (6) PRIMARY KEY, kgrbs varchar (3), title text NOT NULL, tier int4, deleted BOOLEAN NOT NULL);

-- statuses definition

CREATE TABLE statuses (uid VARCHAR NOT NULL PRIMARY KEY, document_type VARCHAR NOT NULL, title VARCHAR NOT NULL, status_type VARCHAR, priority INTEGER, color TEXT) WITHOUT ROWID;

-- plan_documents definition

CREATE TABLE plan_documents (id INTEGER NOT NULL PRIMARY KEY, year_n INTEGER NOT NULL, kgrbs VARCHAR REFERENCES kkrb_kgrbs (code), changes BOOLEAN NOT NULL DEFAULT (True), doc_number INTEGER, doc_date VARCHAR (13), status_uid VARCHAR NOT NULL REFERENCES statuses (uid) ON DELETE CASCADE ON UPDATE CASCADE MATCH SIMPLE, basis_uid VARCHAR REFERENCES bases (uid) ON DELETE CASCADE ON UPDATE CASCADE MATCH SIMPLE, user_comment varchar (255), foreign_uuid VARCHAR, FOREIGN KEY (basis_uid) REFERENCES bases (uid), FOREIGN KEY (kgrbs) REFERENCES kkrb_kgrbs (code), FOREIGN KEY (status_uid) REFERENCES statuses (uid));

-- plan_data definition

CREATE TABLE plan_data (id INTEGER NOT NULL PRIMARY KEY, document_id INTEGER NOT NULL REFERENCES plan_documents (id) ON DELETE CASCADE ON UPDATE CASCADE, month_n INTEGER NOT NULL, amount DOUBLE, krp TEXT REFERENCES kkrb_krp (code), kcs TEXT REFERENCES kkrb_kcs (code), kvr TEXT REFERENCES kkrb_kvr (code), kosgu TEXT REFERENCES kkrb_kosgu (code), kubp TEXT REFERENCES kubp (code), expenses_part TEXT, notes TEXT);

-- expenses_editor_plan definition

CREATE TABLE expenses_editor_plan (id INTEGER PRIMARY KEY, document_id INTEGER NOT NULL REFERENCES plan_documents (id) ON DELETE CASCADE ON UPDATE CASCADE MATCH SIMPLE, krp varchar (4) NOT NULL REFERENCES kkrb_krp (code), kcs varchar (7) NOT NULL REFERENCES kkrb_kcs (code), kvr varchar (3) NOT NULL REFERENCES kkrb_kvr (code), kosgu varchar (3) NOT NULL REFERENCES kkrb_kosgu (code), kubp varchar (6) NOT NULL REFERENCES kubp (code), m_1 DOUBLE NOT NULL DEFAULT (0), m_2 DOUBLE NOT NULL DEFAULT (0), m_3 DOUBLE NOT NULL DEFAULT (0), m_4 DOUBLE NOT NULL DEFAULT (0), m_5 DOUBLE NOT NULL DEFAULT (0), m_6 DOUBLE NOT NULL DEFAULT (0), m_7 DOUBLE NOT NULL DEFAULT (0), m_8 DOUBLE NOT NULL DEFAULT (0), m_9 DOUBLE NOT NULL DEFAULT (0), m_10 DOUBLE NOT NULL DEFAULT (0), m_11 DOUBLE NOT NULL DEFAULT (0), m_12 DOUBLE NOT NULL DEFAULT (0), expenses_part VARCHAR NOT NULL, notes VARCHAR (255));

-- fact_documents definition

CREATE TABLE fact_documents (id INTEGER NOT NULL, year_n INT2 NOT NULL, kgrbs VARCHAR (3), doc_number VARCHAR (32), doc_date TIMESTAMP (13) NOT NULL, status_uid VARCHAR NOT NULL REFERENCES statuses (uid) ON DELETE CASCADE ON UPDATE CASCADE MATCH SIMPLE, payment_type TEXT NOT NULL, user_comment VARCHAR (255), foreign_uuid VARCHAR, funded_month INTEGER AS (strftime('%m', doc_date)), CONSTRAINT FACT_DOCUMENTS_PK PRIMARY KEY (id), CONSTRAINT FK_fact_documents_kkrb_kgrbs FOREIGN KEY (kgrbs) REFERENCES kkrb_kgrbs (code));

-- fact_data definition

CREATE TABLE fact_data (id INTEGER NOT NULL, document_id INT4 NOT NULL, krp VARCHAR (4), kcs VARCHAR (7), kvr VARCHAR (3), kosgu VARCHAR (3), kubp VARCHAR (6), notes VARCHAR (255), direction VARCHAR (255), expenses_part VARCHAR, amount NUMERIC (18, 2), CONSTRAINT FACT_DATA_PK PRIMARY KEY (id), CONSTRAINT FK_fact_data_fact_documents_2 FOREIGN KEY (document_id) REFERENCES fact_documents (id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT FK_fact_data_kkrb_kcs_3 FOREIGN KEY (kcs) REFERENCES kkrb_kcs (code), CONSTRAINT FK_fact_data_kkrb_kosgu_4 FOREIGN KEY (kosgu) REFERENCES kkrb_kosgu (code), CONSTRAINT FK_fact_data_kkrb_krp_5 FOREIGN KEY (krp) REFERENCES kkrb_krp (code), CONSTRAINT FK_fact_data_kkrb_kvr_6 FOREIGN KEY (kvr) REFERENCES kkrb_kvr (code), CONSTRAINT FK_fact_data_kubp_7 FOREIGN KEY (kubp) REFERENCES kubp (code));

-- expenses_editor_fact definition

CREATE TABLE expenses_editor_fact (id INTEGER, document_id INTEGER NOT NULL, krp VARCHAR (4) NOT NULL, kcs VARCHAR (7) NOT NULL, kvr VARCHAR (3) NOT NULL, kosgu VARCHAR (3) NOT NULL, kubp VARCHAR (6) NOT NULL, notes VARCHAR (255), direction TEXT, expenses_part VARCHAR, amount DOUBLE DEFAULT 0 NOT NULL, CONSTRAINT EXPENSES_EDITOR_FACT_PK PRIMARY KEY (id), CONSTRAINT FK_expenses_editor_fact_fact_documents FOREIGN KEY (document_id) REFERENCES fact_documents (id) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT FK_expenses_editor_fact_kkrb_kcs_2 FOREIGN KEY (kcs) REFERENCES kkrb_kcs (code) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT FK_expenses_editor_fact_kkrb_kosgu_3 FOREIGN KEY (kosgu) REFERENCES kkrb_kosgu (code) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT FK_expenses_editor_fact_kkrb_krp_4 FOREIGN KEY (krp) REFERENCES kkrb_krp (code) ON DELETE CASCADE ON UPDATE CASCADE, CONSTRAINT FK_expenses_editor_fact_kkrb_kvr_5 FOREIGN KEY (kvr) REFERENCES kkrb_kvr (code) ON DELETE CASCADE ON UPDATE SET DEFAULT, CONSTRAINT FK_expenses_editor_fact_kubp_6 FOREIGN KEY (kubp) REFERENCES kubp (code) ON DELETE CASCADE ON UPDATE CASCADE);

