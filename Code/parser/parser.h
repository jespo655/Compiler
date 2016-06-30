#pragma once

// #include "../abstx/abstx.h"
// #include "../abstx/workspace.h"
// #include "token.h"


enum struct Parsing_status
{
    NOT_PARSED,             // not parsed at all.
    PARTIALLY_PARSED,       // parsed to the point that it's apparent what type of expression it is.
    FULLY_RESOLVED,         // the expression and all sub-expressions are fully parsed without errors.

    DEPENDENCIES_NEEDED,    // the expression cannot yet be fully parsed because it's waiting for other things to parse first

    // Error codes: the expression cannot be fully parsed because:
    SYNTAX_ERROR,           // there was a syntax error
    TYPE_ERROR,             // some types doesn't match
    CYCLIC_DEPENDENCY,      // there was a cyclic dependency
    COMPILE_TIME_ERROR,     // there was an error in a #run (that was not one of the above error types)
    FATAL_ERROR,            // there was an error so bad that we couldn't recover. Continued compilation would give undefined behaviour.

    // TODO: Add more error types as they pop up

};

static public is_error(Parsing_status p) {
    if (p == Parsing_status::NOT_PARSED
        || p == Parsing_status::PARTIALLY_PARSED
        || p == Parsing_status::FULLY_RESOLVED
        || p == Parsing_status::DEPENDENCIES_NEEDED)
        return false;
    return true;
}

static public is_fatal(Parsing_status p) {
    return p == Parsing_status::FATAL_ERROR;
}



/*
Pass 1: Partial parsing

Alla filer gås igenom och dess static scope parsas partiellt


Varje statement gås igenom mha parentes-checkar (allt som nämns här är endast utanför parenteser)

TILLÅTET I STATIC SCOPE:

Hittar ':' token: declaration
    Allting innan ':' måste vara nydeklarerade identifiers (detta kan kollas)
    Deklarerade identifiers läggs till i local_scope->identifiers
    Inget efter ':' parsas.
    -> gå till nästa ';'

using statement (using "file.h"; using test;)
    Lägg till i en lista av using-statements, resolva dem sist (måste fully resolvas innan scope är partially resolvat)
    Om string literal -> inkludera den filen (kolla först i listan av kompilerade filer)
        öppna den filen, skapa nytt workspace, försök partially resolva det, lägg till global scope i using-listan
    Om identifier -> försök partially resolva den. Om scope identifier, lägg till i using-listan. Om något annat -> type error
    -> gå till ';'

statement som börjar med '{'
    Anonymous scope
    Parsa partiellt sist (efter using)
    -> gå till matchande '}'

statement som börjar med #run
    lägg till i en lista av #run (i workspace)
    lägg inte till som statement i static scope
    kör sist (efter using och efter anonymous scopes, de kan också innehålla #runs) (compile time execution)
    (om det inte finns några #run, avsluta kompileringen med tom output. Entry point måste sättas någonstans.)
    -> gå till ';'




INTE TILLÅTET I STATIC SCOPE: (detta skulle ge SYNTAX_ERROR)

Inget av detta hittas, men expr slutar med en parentes '()' och sen direkt ';': Function call
    Inget parsas.
    -> gå till ';'

return statement
    Inget parsas.
    -> gå till ';'

if, elsif, else, then, for, while
    Kräv följande '()' (där det är relevant) och '{}'-scope
    -> gå till matchande '}''

Hittar '=' token: assignment
    Inget parsas.
    -> gå till nästa ';'

Inget av detta, men följer operator mönster:
    expr @ expr; -> infix_operator
    expr @; -> suffix_operator
    @ expr; -> prefix_operator
        (där @ är en identifier eller symbol token)
    Ger syntax error i pass 1 ("unable to parse statement"), men borde kunna tolkas och parsas fullt direkt i pass 2 och 3
    -> gå till ';'

Vad som helst annat:
    Syntax error
    -> gå till ';'




Pass 2: #run

Alla #run från global scope körs

Ett statement i taget parsas helt
Sätt upp dependencies om något inte går att parsa, och parsa dem först.
    Dependencies behövs endast för att undvika cykler
    Om en cyklisk dependency hittas -> log error (CYCLIC_DEPENDENCY)

Interpretera koden och kör den compile time, en funktion i taget.
    Runtime error -> fånga och log error (COMPILE_TIME_ERROR)

När alla #runs är klara, se till att output entry point är satt
    Annars -> log error (COMPILE_TIME_ERROR)




Pass 3: output

Utgå från output entry point
Kör som i pass 2, men istället för att evaluera allt, översätt till c-kod och skriv till fil
Fully resolva ett statement i taget och översätt. Allt borde vara mycket likt pass 2.

*/





/*
The parser builds an abstract syntax tree from a set of tokens
*/



/*
Problem:

for incremental compilation, each statement has to know about its own list of tokens (unless finalized)

    Each abstx has a start token pointer
    To iterate through tokens, construct a token iterator with this pointer

    The list of tokens end with eof token
    The iterator cannot go past this token

    Make sure that the list of tokens doesn't deallocate or reallocate
        it cannot change size
        look out for possible reallocations / heap cleanup (does that even happen in c++?)
*/



/*

Partial parsing paren mismatch recovery:

Gå tillbaka till början av statement
Försök fully resolva statement
Detta kan ge bra felmeddelanden med sammanhang:

ex:
    int a = ); // "unexpected ')' in rhs of assignment"

Om det går att städa upp, gör det och skicka tillbaka SYNTAX_ERROR. Om inte, returnera FATAL_ERROR.




i c++:

    {} matchar alltid, oberoende av potentiella fel mellan
    vid en extra ')': gå till nästa ';' eller '}', men fortsätt matcha scopes {}

*/