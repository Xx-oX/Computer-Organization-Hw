.text
	main:
		addi $a0, $zero, 10 
		jal f
		syscall
		
	f:
		#if $a0 > 1 => Recurrsion
		addi $t1, $zero, 1
		slt $at, $t1, $a0   
		bne $at, $zero, Recurrsion
		
		addi $v0, $a0, 0
		jr $ra	
			
		Recurrsion:
			addi $sp, $sp, -12 #get stack
			sw $ra, 0($sp) #save $ra
			
			sw $a0, 4($sp) #save n	
			addi $a0, $a0, -1 #$a0 -= 1	
			jal f
			lw $a0, 4($sp) #restore $a0 = n
			sw $v0, 8($sp) #save result of f(n-1)
		
			addi $a0, $a0, -2 #$a0 -= 2
			jal f
			lw $t0, 8($sp) #restore $to = f(n-1)
			add $v0, $v0, $t0 #return value
			
			lw $ra, 0($sp) #restore $ra
			addi $sp, $sp, 12 #return stack
			jr $ra
