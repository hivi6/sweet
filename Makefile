CPP := g++
EXE := sweet

main:
	${CPP} main.cpp -o ${EXE}

clean:
	rm ${EXE}