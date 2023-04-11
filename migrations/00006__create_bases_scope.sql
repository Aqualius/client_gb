CREATE TABLE bases_scope (
    id INTEGER,
    basis_id INTEGER NOT NULL,
    doc_number VARCHAR(32) NOT NULL,
    kgrbs VARCHAR(3) NOT NULL,
    CONSTRAINT pk_expenses_bases_scope PRIMARY KEY (id),
    CONSTRAINT expenses_bases_scope_un UNIQUE (basis_id, doc_number, kgrbs),
    CONSTRAINT expenses_bases_scope_kgrbs_fk FOREIGN KEY (kgrbs) REFERENCES kkrb_kgrbs(code),
    CONSTRAINT expenses_bases_scope_basis_fk FOREIGN KEY (basis_id) REFERENCES bases(uid) ON DELETE CASCADE
);