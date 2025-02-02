#include<iostream>
#include<string>
#include<bitset>
#include<vector>
#include<math.h>
#include<random>

using namespace std;

vector<vector<int>> datablock;
vector<vector<int>> datablockWithCheckBits;
vector<int> columnWiseSerial;
vector<int> columnWiseSerialWithCRC;
vector<int> errorPoints;
vector<vector<int>> errorPointsBlock;
string genPoly;

int generateRandomNumber(int minValue, int maxValue) {
    std::random_device rd;  // Seed generator
    std::mt19937 mt(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<int> dist(minValue, maxValue);

    // Generate and return the random number
    return dist(mt);
}

string paddingTheString(int numberOfBytesInARow, string str){
    string result;
    int lengthOfString = str.length();
    // int reminder = lengthOfString % numberOfBytesInARow;
    // reminder = numberOfBytesInARow - reminder;
    int reminder = lengthOfString;
    while(reminder % numberOfBytesInARow){
        reminder++;
    }
    reminder-=lengthOfString;
    if(!reminder) return str;

    result = str;
    for(int i=0;i<reminder;i++){
        result.append("~");
    }

    return result;
}

string charToAsciiBinary(char character) {
    // Convert the character to its ASCII value
    int asciiValue = static_cast<int>(character);

    // Convert the ASCII value to binary representation
    string binary = bitset<8>(asciiValue).to_string();

    return binary;
}

char binaryToChar(const std::string& binaryAscii) {
    if (binaryAscii.size() != 8) {
        // Check if the input string is exactly 8 bits long
        throw std::invalid_argument("Input binary string must be 8 bits long.");
    }

    // Convert the binary string to an integer
    int asciiValue = std::bitset<8>(binaryAscii).to_ulong();

    // Convert the integer to a char
    return static_cast<char>(asciiValue);
}

int numberOfParitybits(int m){
    int r=0;
    while (m + r + 1 > (1 << r)) {
        r++;
    }
    return r;
}

void init(string dataStr, int m){
    int nRows = dataStr.length() / m;
    int itr = 0;
    for(int i=0; i<nRows; i++){
        vector<int> temp;
        vector<int> errorTemp;
        for(int j=0; j<m ; j++){
            string asciiValue = charToAsciiBinary(dataStr[itr]);

            for(int iiii=0; iiii<asciiValue.length(); iiii++){
                temp.push_back(asciiValue[iiii]-'0');
                errorTemp.push_back(0);
            }

            itr++;
        }
        datablock.push_back(temp);
        errorPointsBlock.push_back(errorTemp);
        
    }

    for(int i=0; i<nRows; i++){
        vector<int> temp;
        int r = numberOfParitybits(m);
        for(int j=0; j<m*( 8 + r); j++){
            temp.push_back(0);
        }
        datablockWithCheckBits.push_back(temp);
    }
}

void printVec(bool checkBits,bool isGreen,bool isRed){
     
    if(!checkBits){
        for(vector<int> temp : datablock){
            for(int t : temp){
                cout<<t;
            }
            cout<<endl;
        }
    }else{
        int idx = 1;
        int j=0;

       

        for(vector<int> temp : datablockWithCheckBits){
            
            for(int i=0; i<temp.size(); i++){
                //cout<<"here"<<endl;
                if(idx-1 == i && isGreen){
                    cout << "\x1b[32m" << temp[i] << "\x1b[0m";
                    idx=idx<<1;
                }else if(isRed && errorPointsBlock[j][i]){
                    cout << "\x1b[31m" << temp[i] << "\x1b[0m";
                }
                else{
                    cout<<temp[i];
                }
            }
            idx=1;
            cout<<endl;
            j++;
        }
    }
}
vector<int> hammingEncode(const vector<int>& data){
    int m = data.size();
    int r = numberOfParitybits(m);
    int n = m + r;

    vector<int> encodedMsg(n);
    
    //cout<<m<<" "<<r<<" "<<n<<endl;
    //marking the places for parity bits
    for(int i=0; i<r; i++){
        encodedMsg[pow(2,i)-1] = -1;
    }

    //placing the data bits 
    int idx = 0;
    for(int i=0;i<n;i++){
        if(encodedMsg[i]==-1) continue;

        encodedMsg[i]=data[idx];
        idx++;
    }

    //calculating the parity bits
    for(int i=0; i<n; i++){
        if(encodedMsg[i]==-1){
            int x = log2(i+1);
            int nOnes = 0;

            for(int j=i+1; j<n; j++){
                if((j+1) & (1<<x)){
                    if(encodedMsg[j]==1){
                        nOnes++;
                    }
                }
            }

            encodedMsg[i] = (nOnes % 2 == 0)? 0 : 1;
        }else{
            //do nothing
        }
    }

    return encodedMsg;
}

int hammingDecode(const vector<int>& data){
    
    int n = data.size();
    //int r = numberOfParitybits(m);
    // int n = m + r;
    string ans = "";
    int bit = 0;
    for(int i=1; i<=n ; i=i*2){
        int cnt = 0;
        if(data[i-1]==1) cnt++;

        //cout<<"kader ney vai"<<endl;
        for(int j=i+1;j<=n;j++){
            if(j & 1<<bit){
                //cout<<j<<" ";
                if(data[j-1]==1) cnt++;
            }
            //cout<<endl;
        }

       // cout<<i<<" :"<<cnt<<endl;
        if(cnt%2==0){
            ans.push_back('0');
        }else{
            ans.push_back('1');
        }
        bit++;
        //cout<<"ANS: "<<ans<<endl;
    }
    //cout<<"ANS: "<<ans<<endl;

    int power=1;
    int wrongIndex = 0;
    for(int i=0;i<ans.size();i++){
        //cout<<"ans[i]="<<ans[i]<<endl;
        if(ans[i]=='1'){ wrongIndex = wrongIndex + power; }
        power = power * 2;

        //cout<<"Wrong index: "<<wrongIndex<<endl;
    }
    //cout<<wrongIndex<<endl;
    return wrongIndex;

}

void buildHammingEncodedVec(){
    datablockWithCheckBits.clear();
    for(vector<int> dataLine : datablock){
        vector<int>encodedDataLine = hammingEncode(dataLine);
        datablockWithCheckBits.push_back(encodedDataLine);
    }
   
}

void columnWiseSerialize(){
    columnWiseSerial.clear();
    for(int j=0;j<datablockWithCheckBits[0].size();j++){
        for(int i=0;i<datablockWithCheckBits.size();i++){
            columnWiseSerial.push_back(datablockWithCheckBits[i][j]);
            errorPoints.push_back(0);
        }
    }
}

void printVec(vector<int> vec,bool isCyan,bool isRed){
    for(int i=0;i<vec.size();i++){
        if(isCyan && i > vec.size()-genPoly.size()){
            cout<<"\x1b[36m" << vec[i] << "\x1b[0m";
        }else if(isRed && errorPoints[i]){
            cout<<"\x1b[31m" << vec[i] << "\x1b[0m";
        }
        else{
            cout<<vec[i];
        }
    }
    cout<<endl;
}



//both a and b are same size
string xorString(string a, string b)
{
 
    string result = "";
 
    int n = b.length();
    
    //starting from 1 to leave out the first digit
    for (int i = 1; i < n; i++) {
        if (a[i] == b[i])
            result += "0";
        else
            result += "1";
    }
    return result;
}

string divString(string dividend, string divisor){
    int take = divisor.length();
    int sizeOfDividend = dividend.length();
    string sub = dividend.substr(0, take);
    
    while(take<sizeOfDividend){
        if(sub[0]=='1'){
           
            sub = xorString(sub,divisor) + dividend[take];
           
        }else{
            string allZeros = string(divisor.length(),'0');
            
            sub = xorString(sub,allZeros) + dividend[take];
            
        }
        
        take++;
    }
    

    //for last bits "take" value will be higher than dividend.length so it will cause indexOutOfBound

    if(sub[0]=='1'){
        sub = xorString(sub,divisor);
    }else{
        string allZeros = string(divisor.length(),'0');
        sub = xorString(sub,allZeros);
    }
    
    return sub;
}

void CRCencode(const vector<int>& data, string genPoly){

    string dataStr = "";
    for(int x : data){
        dataStr+= x + '0';
    }
    
    string appendedDataStr = dataStr + string(genPoly.size()-1,'0');
    string reminder = divString(appendedDataStr,genPoly);
    
    columnWiseSerialWithCRC = columnWiseSerial;
    for(char c: reminder){
        columnWiseSerialWithCRC.push_back(c-'0');
        errorPoints.push_back(0);
    }

}

bool checkErrorUsingCRC(const vector<int>& data){
    string dataStr = "";
    for(int x: data){
        dataStr+= x +'0';
    }
    
    string remainder = divString(dataStr, genPoly);
    //cout<<"Reminder:"<<remainder<<endl;
    if(string(remainder.size(),'0') !=remainder){
        return true;
    }else{
        return false;
    }
}

void removeCRC(vector<int> data){
    vector<int> temp;
    vector<int> errorTemp;
    for(int i=0;i<data.size();i++){
        if(i > data.size()-genPoly.size()){
            continue;
        }else{
            temp.push_back(data[i]);
            errorTemp.push_back(errorPoints[i]);
        }
    }
    columnWiseSerialWithCRC = temp;
    errorPoints = errorTemp;
}

void buildDataBlockFromColWiseSerial(vector<int> data){
   // vector<vector<int>> temp;
   for(int i=0; i<datablockWithCheckBits.size(); i++){
    datablockWithCheckBits[i].clear();
    errorPointsBlock[i].clear();
   }


    for(int i=0; i<data.size();){
        for(int j=0; j<datablockWithCheckBits.size();j++){
            datablockWithCheckBits[j].push_back(data[i]);

            if(errorPoints[i]) errorPointsBlock[j].push_back(1);
            else errorPointsBlock[j].push_back(0);

            i++;
        }
    }
}

void buildDataBlockRemovingCheckBits(vector<vector<int>> data){
    for(int i=0;i<datablock.size();i++){
        datablock[i].clear();
    }



    int idx = 1;
    for(int i=0; i<data.size();i++){
        
        for(int j=0; j<data[i].size(); j++){
            //cout<<"here"<<endl;
            if(idx-1 == j){
                idx = idx<<1;
                continue;
            }else{
                datablock[i].push_back(data[i][j]);
            }
        }
        idx=1;
    }
}

string datablockToStr(vector<vector<int>> data){
    string frame;
    //int i=0;
    string ascii;
    for(int i=0;i<data.size();i++){
        for(int j=0; j<data[i].size();){
            for(int k=0;k<8;k++){
                char digit = data[i][j]+'0';
                ascii+=digit;
                //ascii.append(data[i][j]+'0'+"");
                j++;
            }
            //cout<<ascii<<endl;
            char c = binaryToChar(ascii);
            frame += c;
            ascii.clear();
        }
    }
    return frame;
}

void errorMaker(double p){
 
    

    for(int i=0; i<errorPoints.size(); i++){
        int randNumber = generateRandomNumber(0,1000);
        if(randNumber<p) errorPoints[i] = 1;
    }
}

void insertError(){
    for(int i=0;i<columnWiseSerialWithCRC.size();i++){
        if(errorPoints[i]) columnWiseSerialWithCRC[i] = 1 - columnWiseSerialWithCRC[i];
    }
}
int main(){
    //srand(time());
    string str;
    int numberOfBytesInARow;
    double p = 0.04 * 1000;
    genPoly = "1010";
    cout<<"Enter data string: ";
    getline(cin,str);
    cout<<"Enter number of data bytes in a row(m) :";
    cin>>numberOfBytesInARow;
    cin.ignore();
    cout<<"Enter probability(p) :";
    cin>>p;
    cin.ignore();
    p = p * 1000;
    cout<<"Enter generator polynomial: ";
    getline(cin,genPoly);
    cout<<str<<" "<<numberOfBytesInARow<<" "<<p<<" "<<genPoly<<endl;

    string dataStr = paddingTheString(numberOfBytesInARow, str);
    cout<<"data string after padding: "<<dataStr<<endl;
    init(dataStr,numberOfBytesInARow);

    cout<<"datablock:"<<endl;
    printVec(false,false,false);
    cout<<endl<<endl<<endl;


    cout<<"datablock after adding checkbits"<<endl;
    buildHammingEncodedVec();
    printVec(true,true,false);
    cout<<endl<<endl<<endl;


    cout<<"data bits after column-wise serialization:"<<endl;
    columnWiseSerialize();
    //columnWiseSerialize();
    printVec(columnWiseSerial,false,false);
    cout<<endl<<endl<<endl;



    cout<<"data bits after appending CRC checksum (sent frame):"<<endl;
    CRCencode(columnWiseSerial,genPoly);
    printVec(columnWiseSerialWithCRC,true,false);
    cout<<endl<<endl<<endl;

    cout<<"received frame:"<<endl;
    errorMaker(p);
    insertError();
    printVec(columnWiseSerialWithCRC,false,true);


    cout<<"result of crc checksum matching: ";
    bool isError = checkErrorUsingCRC(columnWiseSerialWithCRC);
    if(isError) cout<<"error detected"<<endl;
    else cout<<"no error found"<<endl;
    cout<<endl<<endl<<endl;

    cout<<"data block after removing CRC checksum bits: "<<endl;
    removeCRC(columnWiseSerialWithCRC);
    buildDataBlockFromColWiseSerial(columnWiseSerialWithCRC);
    printVec(true, false, true);
    cout<<endl<<endl<<endl;

    
    for(int i=0; i<datablockWithCheckBits.size();i++){
        int wrongBit = hammingDecode(datablockWithCheckBits[i]);
        if(wrongBit!=0){
            datablockWithCheckBits[i][wrongBit-1]=1-datablockWithCheckBits[i][wrongBit-1];
        }
        //cout<<"wrongbit: "<<wrongBit<<endl;
    }

 

    cout<<"data block after removing check bits:"<<endl;
    buildDataBlockRemovingCheckBits(datablockWithCheckBits);
    printVec(false,false, false);

    cout<<"output frame: ";
    string frame = datablockToStr(datablock);
    cout<<frame<<endl;

}

// int main(){
//     cout<<"PlayGround"<<endl;
//     vector<int> dataOne = {1,0,0,1,0,0};
//     vector<int> dataTwo = {0,0,0,1,1,1,0,1,1,1,0,1};
    
//     //vector<int> temp = hammingEncode(dataTwo);
    
//     // for(int i=0;i<temp.size();i++){
//     //     cout<<temp[i];
//     // }
//     // cout<<endl;
//     // temp[11]=1;
//     int ans = hammingDecode(dataTwo);
//     cout<<ans<<endl;
//     cout<<endl;
//     return 0;
// }