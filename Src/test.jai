




plus := fn(a:int, b:int)->int { return a+b; };
a := plus(1, 2);

a := 1 plus 2;


bool b = !true; // b = false;

i = 1;
int a = ++i; // i = 2, a = 2

i = 1;
int a = i++; // i = 2, a = 1



plusplus := 2;
not = infix_operator(a:int, b:int)->int { a+b; }
a := (a,1) plus (plusplus not b, 2) plus (c, 2);

not = infix_operator((a:int, b:int), (c:int, d:int))->(int, int) { return a+b, c+d; }



a := 1 plus 2 + 3 plus 4; = 10







infix:
 a = 1 + 2;

prefix:
 a = ++i;

suffix:
 a = i++;




// går inte pga redefinition of a
// a := fn(int, int)->int;
// a := fn(float, float)->float;



int operator+(int a, int b) { return a+b; }
float operator+(float a, float b) { return a+b; }


a, b, c : int = 1;

a : float = 1.0;
a : int = 1.5;



|a|


a @ a
@ a
a @



a + abs b+a + c




val_expr

val_expr @ val_expr // infix
@ val_expr // prefix
val_expr @ // suffix

val_expr()












hej := int;
hej = float;



/// FIL 1

foo := fn(a:float, b:int)->int { return a+b; };

infix_operator + := foo;
infix_operator + := fn(a:float, b:int)->int { return a+b; };



infix_operator + := infix_operator(prio=100);

infix_operator + := foo;
infix_operator + := fn(a:float, b:int)->int { return a+b; }, 100;
infix_operator + := fn(a:float, b:int)->int { return a+b; }, 100;


// infix_operator_declaration_statement
// infix_declaration


{

    infix_operator + := infix_operator(prio=100);

    infix_operator + = fn(a:float, b:int)->int { return a+b; };

    a := 1;

    {
        infix_operator + := infix_operator(prio=100);
        infix_operator + = fn(a:float, b:int)->int { return a-b; };

        print(a);

        a := "asd";

        print(a);

    }

    f + i;

}














/// FIL 2

infix_operator + := op(int a, int b)->int { return a+b; };







infix_operator + = op(int a, int b)->int { return a+b; };





infix_operator asd := op(int a, int b)->int { return a+b; };
infix_operator asd = op(float a, float b)->int { return a+b; };
infix_operator asd = op(float a, int b)->int { return a+b; };
infix_operator asd = op(string a, int b)->int { return a+b; };
infix_operator asd = op(string a, string b)->int { return a+b; };

// infix_operator asd = op(int a, int b)->int { return a+b; };





infix_operator+ := op(float a, float b)->float { return a+b; }



asd Type_infix_op










