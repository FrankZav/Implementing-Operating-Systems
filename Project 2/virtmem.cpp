
#include <iostream>
#include <bitset>
#include <fstream>
#include <sstream>
#include <string>

int totalEntries = 0;
int* bitmap = new int[32];
int * PA = new int [524288];
int MASK[32];
int MASK2[32];

struct TLB{
	int LRU[4] = {0, 1, 2, 3};
	int sp[4];
	int faddr[4];
};

TLB ourTLB;
int setbit(int frame);

int decipherforS(int address){
	address = address >> 19;
	int S = 0;
	for (int i = 0; i < 9; i++)
	{
		S += address & MASK[31 - i];
	}
	return S;
}
int decipherforP(int address){
	address = address >> 9;
	int P = 0;
	for (int i = 0; i < 10; i++)
	{
		P += address & MASK[31 - i];
	}
	return P;
}
int decipherforSP(int address){
	int SP = decipherforS(address);
	SP = SP<<10;
	SP += decipherforP(address);
	return SP;
}
int decipherforW(int address){
	int W = 0;
	for (int i = 0; i < 9; i++)
	{
		W += address & MASK[31 - i];
	}
	return W;
}

int runlookupSegment (int address){
	int segmentnum = decipherforS(address);
	if (PA[segmentnum] == -1)
		return -1;
	else if(PA[segmentnum] == 0)
		return 0;
	else{
		int PT = PA[segmentnum];
		return PT;
	}
}

int TLBlookup(int address){
	for (int i = totalEntries-1; i>=0;i--){
		if (ourTLB.sp[i] == decipherforSP(address)){
			for(int j = 0;j<=3;j++){
				if (ourTLB.LRU[i]<ourTLB.LRU[j])
					ourTLB.LRU[j] -=1;
			}
			ourTLB.LRU[i] = 3;
			return ourTLB.faddr[i];
		}
	}
	return -1;
}


int findfreeframes(int num){
	for (int i = 0; i< 32; i++){
		for (int j = 31; j>=0; j--){
			if(num == 1){
				if ((MASK[j] & bitmap[i])==0){
					bitmap[i] = MASK[j]|bitmap[i];
					return (i*32)+(31-j);
				}
			}
			else{
				if (j == 0 && i<31){
					if((MASK[j] & bitmap[i]) ==0){
						if ((MASK[31] & bitmap[i+1]) ==0){
							bitmap[i] = MASK[j] | bitmap[i];
							bitmap[i+1] =  MASK[31] | bitmap[i+1];
							return (i*32)+(31-j);
						}
					}
				}
				else if ((MASK[j]&bitmap[i]) == 0){
					if ((MASK[j-1]&bitmap[i]) == 0){
						bitmap[i] = MASK[j] | bitmap[i];
						bitmap[i] =  MASK[j-1] | bitmap[i];
						return (i*32)+(31-j);
					}
				}
			}
		}
	}
	return -1;
}

int initializePages(int page,int segmentnum, int paddress){
	PA[PA[segmentnum] + page] = paddress;
	if (paddress != -1)
		setbit(paddress/512);
	return paddress;
}

int setbit(int frame){
	int i  = frame/32;
	int j = frame - (i*32);
	bitmap[i] = bitmap[i] | MASK[31-j];
	return bitmap[i];
}

int read(int address){
	int PT = runlookupSegment(address);
	if (PT <=0)
		return PT;
	else{
		int PTE = PT + decipherforP(address);
		if (PA[PTE]<=0)
			return PA[PTE];
		else
			return PA[PTE] + decipherforW(address);
	}
}

int write(int address){
	int PT = runlookupSegment(address);
	if (PT == -1)
		return PT;
	if (PT == 0){
		PA[decipherforS(address)] = findfreeframes(2)*512;
	}
	PT = runlookupSegment(address)+ decipherforP(address);
	if (PA[PT] ==-1)
		return -1;
	if (PA[PT] == 0){
		PA[PT] = findfreeframes(1) * 512;
	}
	return PA[PT]+decipherforW(address);

}

void shell(){
	std::ifstream initial("E:\\input1.txt");
	std::ifstream ourinput("E:\\input2.txt");
	std::ofstream ouroutput("E:\\62706684.txt");
	std::string buf;
	std::getline(initial, buf);
	std::string a = "";
	std::string b = "";
	int counter = 0;
	for (int i =0; i<buf.length();i++){
		if (buf[i] == ' ')
			counter++;
		else if (counter == 0)
			a.push_back(buf[i]);
		else if (counter == 1)
			b.push_back(buf[i]);
		if (counter >1 || i == buf.length()-1){
			PA[stoi(a)] = stoi(b);
			counter = 0;
			setbit(stoi(b)/512);
			setbit(stoi(b)/512+1);
			a="",b = "";
		}
	}
	buf = "";
	std::getline(initial, buf);
	a = "";
	b = "";
	std::string c =  "";
	counter = 0;
	for (int i =0; i<buf.length();i++){
		if (buf[i] == ' ')
			counter++;
		else if (counter == 0)
			a.push_back(buf[i]);
		else if (counter == 1)
			b.push_back(buf[i]);
		else if (counter == 2)
			c.push_back(buf[i]);
		if (counter >2 || i == buf.length()-1){
			initializePages(stoi(a),stoi(b),stoi(c));
			counter = 0;
			a="",b="",c = "";
		}
	}
	buf = "";
	std::getline(ourinput, buf);
	a= "", b = "";
	counter = 0;
	int toOutput = 0;
	for (int i =0; i<buf.length();i++){
		if (buf[i] == ' ')
			counter++;
		else if (counter == 0)
			a.push_back(buf[i]);
		else if (counter == 1)
			b.push_back(buf[i]);
		if (counter >1 || i == buf.length()-1){
			if (stoi(a) == 0){
				toOutput = read(stoi(b));
			}
			else if (stoi(a) == 1){
				toOutput = write(stoi(b));
			}
			if (toOutput == 0)
				ouroutput<<"err ";
			else if (toOutput == -1)
				ouroutput<<"pf ";
			else
				ouroutput << toOutput<< " ";
			a="",b = "";
			counter = 0;
		}
	}
}

void TLBshell(){
	std::ifstream initial("E:\\input1.txt");
	std::ifstream ourinput("E:\\input2.txt");
	std::ofstream ouroutput("E:\\62706684_tlb.txt");
	std::string buf;
	std::getline(initial, buf);
	std::string a = "";
	std::string b = "";
	int counter = 0;
	for (int i =0; i<buf.length();i++){
		if (buf[i] == ' ')
			counter++;
		else if (counter == 0)
			a.push_back(buf[i]);
		else if (counter == 1)
			b.push_back(buf[i]);
		if (counter >1 || i == buf.length()-1){
			PA[stoi(a)] = stoi(b);
			counter = 0;
			setbit(stoi(b)/512);
			setbit(stoi(b)/512+1);
			a="",b = "";
		}
	}
	buf = "";
	std::getline(initial, buf);
	a = "";
	b = "";
	std::string c =  "";
	counter = 0;
	for (int i =0; i<buf.length();i++){
		if (buf[i] == ' ')
			counter++;
		else if (counter == 0)
			a.push_back(buf[i]);
		else if (counter == 1)
			b.push_back(buf[i]);
		else if (counter == 2)
			c.push_back(buf[i]);
		if (counter >2 || i == buf.length()-1){
			initializePages(stoi(a),stoi(b),stoi(c));
			counter = 0;
			a="",b="",c = "";
		}
	}
	buf = "";
	std::getline(ourinput, buf);
	a= "", b = "";
	counter = 0;
	int toOutput = 0;
	for (int i =0; i<buf.length();i++){
		if (buf[i] == ' ')
			counter++;
		else if (counter == 0)
			a.push_back(buf[i]);
		else if (counter == 1)
			b.push_back(buf[i]);
		if (counter >1 || i == buf.length()-1){
			std::cout<<std::endl;
			toOutput = TLBlookup(stoi(b));
			if (toOutput != -1){
				ouroutput<<"h "<<(toOutput+decipherforW(stoi(b)))<<" ";
				a="",b = "";
				counter = 0;
			}
			else{
				ouroutput<<"m ";
				if (stoi(a) == 0){
					toOutput = read(stoi(b));
				}
				else if (stoi(a) == 1){
					toOutput = write(stoi(b));
				}
				if (toOutput == 0)
					ouroutput<<"err ";
				else if (toOutput == -1)
					ouroutput<<"pf ";
				else{
					if (totalEntries <4) {
						ourTLB.LRU[totalEntries] = 3;
						ourTLB.sp[totalEntries]  = decipherforSP(stoi(b));
						ourTLB.faddr[totalEntries] = PA[PA[decipherforS(stoi(b))]+decipherforP(stoi(b))];
						for (int j = 0; j<=3;j++){
							if (j!=totalEntries)
								ourTLB.LRU[j] -=1;
						}
						totalEntries++;
					}
					else{
						for(int i = 0; i<=3;i++){
							if (ourTLB.LRU[i] == 0){
								ourTLB.LRU[i] = 3;
								ourTLB.sp[i]  = decipherforSP(stoi(b));
								ourTLB.faddr[i] = PA[PA[decipherforS(stoi(b))]+decipherforP(stoi(b))];
								for (int j = 0; j<=3;j++){
									if (j!=i)
										ourTLB.LRU[j] -=1;
								}
								break;
							}
						}
					}
					ouroutput << toOutput<< " ";

				}
				a="",b = "";
				counter = 0;
			}
		}
	}
}

int main() {
	int allowTLB = 1;
	for (int i = 31; i>=0; i--){
		if ( i == 31){
			MASK[i] = 1;
			MASK2[i] = ~MASK[i];
		}
		else{
			MASK[i] = MASK[i+1]<<1;
			MASK2[i] = ~MASK[i];
		}
	}
	bitmap[0] = 1;
	if (allowTLB ==0)
		shell();
	else
		TLBshell();

	delete[] bitmap;
	delete[] PA;
	return 0;
}




