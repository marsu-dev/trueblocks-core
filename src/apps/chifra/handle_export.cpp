/*-------------------------------------------------------------------------
 * This source code is confidential proprietary information which is
 * Copyright (c) 2017 by Great Hill Corporation.
 * All Rights Reserved
 *------------------------------------------------------------------------*/
#include "options.h"

//------------------------------------------------------------------------------------------------
bool COptions::handle_export(void) {

    if (addrs.empty())
        return usage("This function requires an address. Quitting...");

    for (auto addr : addrs) {
        if (!freshen_internal(monitorsPath, addr, "", INDEX))
            return false;
        ostringstream os;
        os << "cd " << monitorsPath << " ; ";
        os << "grabABI " << addr << " ; ";
        os << "acctExport" << " --for_addr " << addr << " " << flags;
        if (isTestMode())
            cout << substitute(os.str(), blockCachePath(""), "$BLOCK_CACHE/") << endl;
        else
            if (system(os.str().c_str())) { }  // Don't remove. Silences compiler warnings
    }
    return true;
}
