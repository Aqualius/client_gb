
CREATE TEMPORARY TABLE bases_backup (
    uid          VARCHAR NOT NULL
                         PRIMARY KEY,
    basis_type   VARCHAR,
    basis_number VARCHAR,
    basis_date   VARCHAR NOT NULL,
    with_changes BOOLEAN NOT NULL
                         DEFAULT (false) 
)
WITHOUT ROWID;

INSERT INTO bases_backup SELECT uid, basis_type, basis_number, basis_date, with_changes FROM bases;

DROP TABLE bases;

CREATE TABLE bases (
    uid          VARCHAR NOT NULL
                         PRIMARY KEY,
    basis_type   VARCHAR,
    basis_number VARCHAR,
    basis_date   VARCHAR NOT NULL,
    with_changes BOOLEAN NOT NULL
                         DEFAULT (false) 
)
WITHOUT ROWID;

INSERT INTO bases SELECT uid, basis_type, basis_number, basis_date, with_changes FROM bases_backup;

DROP TABLE bases_backup;
