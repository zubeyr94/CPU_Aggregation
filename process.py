import sys

filepath = sys.argv[1];
out = open(filepath.split('/')[-1][4:], "w");
print(filepath[4:])
with open(filepath) as fp:
    line = fp.readline()
    cnt = 1
    while line:
        line = fp.readline()
        cnt += 1
        cnt %= 6 
        if(cnt==3):
            avg=float(line)
        elif(cnt==4):
            avg+=float(line)
        elif(cnt==5):
            avg+=float(line)
            avg/=3
            out.write(format(avg,".3f") + "\n")


        
