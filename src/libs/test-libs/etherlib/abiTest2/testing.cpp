/*-------------------------------------------------------------------------------------------
 * qblocks - fast, easily-accessible, fully-decentralized data from blockchains
 * copyright (c) 2018, 2019 TrueBlocks, LLC (http://trueblocks.io)
 *
 * This program is free software: you may redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version. This program is
 * distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. You should have received a copy of the GNU General
 * Public License along with this program. If not, see http://www.gnu.org/licenses/.
 *-------------------------------------------------------------------------------------------*/
#include <algorithm>
#include "options.h"

extern void doOne(const string_q& fn);
extern void doTupleTest(void);
extern void doFunctionTest(void);
//--------------------------------------------------------------
int main(int argc, const char* argv[]) {
    nodeNotRequired();
    etherlib_init(quickQuitHandler);

    COptions options;
    if (!options.prepareArguments(argc, argv))
        return 0;

    doTupleTest();
    doFunctionTest();

    return 0;
}

//--------------------------------------------------------------
extern void doTupleTest(void) {
    manageFields("CTransaction:articulatedTx", true);
    manageFields("CLogEntry:articulatedLog", true);
    manageFields("CParameter:type,internalType,components,indexed", false);
    manageFields("CTransaction:hash,blockHash,nonce,timestamp,gas,gasPrice,gasUsed", false);
    manageFields("CLogEntry:logIndex,topics,data", false);
    manageFields("CFunction:type,signature,encoding,outputs,stateMutability", false);
    manageFields("CReceipt:contractAddress,gasUsed,status", false);

    cout << "[";
    doOne("approve");
    cout << ",";
    doOne("donate");
    cout << "]";
}

//--------------------------------------------------------------
bool visitLine(const char* lineIn, void* data) {
    CFunction func;
    func.fromDefinition(lineIn);
    static bool first = true;
    cout << (!first ? ", " : "");
    cout << func << endl;
    first = false;
    return true;
}

//--------------------------------------------------------------
extern void doFunctionTest(void) {
    cout << "[";
    forEveryLineInAsciiFile("./functions.txt", visitLine, NULL);
    cout << "]";
}

//--------------------------------------------------------------
void doOne(const string_q& fn) {
    CAbi abi_spec;
    loadAbiFile("./" + fn + ".json", &abi_spec);
    CTransaction tx;
    string_q txStr = asciiFileToString("./" + fn + ".tx");
    tx.parseJson3(txStr);
    abi_spec.articulateTransaction(&tx);
    for (auto log : tx.receipt.logs) {
        abi_spec.articulateLog(&log);
    }
    cout << tx << endl;
}
