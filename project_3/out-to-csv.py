def convert(s):
	l = s.split("m")
	return str(float(l[0]) * 60 + float(l[1][:-2]))

fr = open("QuickBenchmarks.txt", "r")
fw = open("QuickBenchmarks.csv", "w")

line = fr.readline()
while line:
	if len(line.split("../../tests/")) > 1:
		fw.write(line.split("../../tests/")[1][:-5])
	elif len(line.split("real")) > 1:
		fw.write(", " + convert(line.split("real	")[1]) + "\n")
	line = fr.readline()

fr.close()
fw.close()


