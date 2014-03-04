package jnicli;

/**
 * Constraints format:
 * <pre>
 *   constraints ::= constraint{',' constraint}
 *   constraint ::= field-name [index-constraint] [unique-constraint] [references-constraint] [autoincrement-constraint]
 *   index-constraint ::= 'using' ['thick'] ['case_insensitive'] 'index'
 *   unique-constraint ::= 'unique'
 *   autoincrement-constraint ::= 'autoincrement'
 *   references-constraint ::= 'references' table-name ['(' inverse-field ')']
 * </pre>
 */
class ConstraintParser { 
    int      pos;
    String   constraints;
    String[] fields;
    String[] inverseFields;
    String[] referencedTables;
    int[]    constraintMasks;

    String getToken() { 
        int p = pos;
        int len = constraints.length();
        while (p < len && Character.isWhitespace(constraints.charAt(p))) {
            p += 1;
        }
        if (p == len) { 
            return null;
        }
        if (Character.isJavaIdentifierStart(constraints.charAt(p))) {
            int start = p;
            while (++p < len && (Character.isJavaIdentifierPart(constraints.charAt(p))
                                 || (constraints.charAt(p)=='.')));
            String tkn = constraints.substring(start, p);
            pos = p;
            return tkn;
        } else {
            pos += 1;
            return constraints.substring(p, p+1);
        }
    }

    ConstraintParser(String fields[], String constraints) {
        this.fields = fields;
        this.constraints = constraints;
        referencedTables = new String[fields.length];
        inverseFields = new String[fields.length];
        constraintMasks = new int[fields.length];
        parse();
    }
    
    void parse() { 
        String tkn;
      constraintLoop:
        while ((tkn = getToken()) != null) { 
            for (int i = 0; i < fields.length; i++) { 
                if (tkn.equals(fields[i])) {
                    tkn = getToken();
                    while (tkn != null) { 
                        if (tkn.equals(",")) { 
                            continue constraintLoop;
                        } else if (tkn.equals("using")) { 
                            while (!"index".equals(tkn = getToken())) {
                                if ("thick".equals(tkn)) { 
                                    constraintMasks[i] |= THICK;
                                } else if ("case_insensitive".equals(tkn)) { 
                                    constraintMasks[i] |= CASE_INSENSITIVE;
                                } else { 
                                    throw new CliException("Unknown index constraint '" + tkn + "'");
                                }
                            }
                            constraintMasks[i] |= INDEXED;
                        } else if (tkn.equals("unique")) { 
                            constraintMasks[i] |= UNIQUE;
                        } else if (tkn.equals("autoincrement")) { 
                            constraintMasks[i] |= AUTOINCREMENT;
                        } else if (tkn.equals("references")) { 
                            tkn = getToken();
                            if (tkn == null) { 
                                throw new CliException("Referenced table name expected");
                            }
                            referencedTables[i] = tkn;
                           tkn = getToken();
                            if ("(".equals(tkn)) { 
                                tkn = getToken();
                                if (tkn == null) { 
                                    throw new CliException("Inverse field name expected");
                                }
                                inverseFields[i] = tkn;
                                tkn = getToken();
                                if (!")".equals(tkn)) { 
                                    throw new CliException("')' expected");
                                }
                                tkn = getToken();
                            }
                            continue;
                        } else { 
                            throw new CliException("Unknown constraint '" + tkn + "'");
                        }
                        tkn = getToken();
                    }
                    return;
                }
            }
            throw new CliException("Constrained field '" + tkn + "' not found in class");
        }
    }

    static final int INDEXED = 2;
    static final int CASE_INSENSITIVE = 8;
    static final int UNIQUE = 16;
    static final int AUTOINCREMENT = 32;
    static final int THICK = 64;

    String[] getReferencedTables() {
        return referencedTables;
    }

    String[] getInverseFields() {
        return inverseFields;
    }

    int[] getConstraintMasks() {
        return constraintMasks;
    }
}