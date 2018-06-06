# EECS 322 Compiler Construction

The languages that are compiled go in the following order: L1 -> L2 -> L3 -> IR -> LA -> LB. LB, being the most abstracted, is aimed at being a subset of the C language.


**L1**: The most basic language, uses only assignments and register names. It has three standard library function calls: print, allocate and arrary-error. The entry point of the program is declared as the outermost function. Values need to be encoded. If the most insignifacant bit is 0, then it is memory; if it is 1, then it is a value. The numbers under each function declaration indicate the number of arguments and local variables respectively.

```
// Program to print the number 3
(:main
	(:main
		0 0

		rdi <- 3
		// need to encode the value
		rdi << 1
		rdi += 1

		// print always looks in rdi, we give it 1 as standard procedure
		print 1

	)
)
```

**L2**: Builds upon L1 and introduces variables. This causes the compiler that produces an L1 program from an L2 program to perform spilling and graph coloring. Furthermore, a new standard library call, `stack-arg`, was introduced which takes care of where on the stack to store the argument so the programmer doesn't need to. 

```
// Program to print the number 3
(:main
	(:main
		0 0

		myVar1 <- 3
		// need to encode the value
		myVar1 << 1
		myVar1 += 1

		rdi <- myVar1
		// print always looks in rdi, we give it 1 as standard procedure
		print 1

	)
)
```

**L3**: Changes the calling convention from L2, and allows for functions to take in parameters. This causes the compiler to introduce tree creation and tiling to generate efficient L2 code from an L3 input program. 

```
define :myEqual (p1, p2){ 
	v3 <- p1 = p2
	br :myLabelTrue 
	return 0
	:myLabelTrue
	return 1 
}

define :main () { 
	return myEqual(3,5)
}
```


**IR**: Introduces tuples, multi-dimensional arrays, function pointers and type declarations. Each function is written in basicblocks and must explicitly declare its return type. The compilers main job beyond the new parsing is the linearization of the multi-dimensional arrays. 

```
define void :main (){ 
	:entry
	int64 %myRes
	int64 %v1
	int64 %v2
	%myRes <- call :myF(5) 
	%v1 <- %myRes * 4 
	%v2 <- %myRes + %v1
	return %v2 
}

define int64 :myF (int64 %p1){ 
	:myLabel
	int64 %p1
	int64 %p2
	%p2<-%p1+1 
	return %p2 
}
``` 

**LA**: All values are not encoded so when you use numbers they are interpeted as exactly what you declare. All array store and load boundaries are checked and basic blocks are no longer a requirement. 

```
void main (){ 
	int64 %myRes
	int64 %v1
	int64 %v2
	%myRes <- call myF(5) 
	%v1 <- %myRes * 4 
	%v2 <- %myRes + %v1
	return %v2 
}

int64 myF (int64 %p1){ 
	int64 %p1
	int64 %p2
	%p2<-%p1+1 
	return %p2 
}
```

**LB**:




