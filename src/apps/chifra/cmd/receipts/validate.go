package receipts

/*-------------------------------------------------------------------------------------------
 * qblocks - fast, easily-accessible, fully-decentralized data from blockchains
 * copyright (c) 2016, 2021 TrueBlocks, LLC (http://trueblocks.io)
 *
 * This program is free software: you may redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version. This program is
 * distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. You should have received a copy of the GNU General
 * Public License along with this program. If not, see http://www.gnu.org/licenses/.
 *-------------------------------------------------------------------------------------------*/

import (
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/cmd/root"
	"github.com/spf13/cobra"
)

func Validate(cmd *cobra.Command, args []string) error {
	// if len(args) == 0 {
	// 	return errors.New(fmtError("You must provide at least one valid transaction identifier"))
	// }
	// for _, arg := range args {
	// 	valid, err := validateTxIdentifier(arg)
	// 	if !valid || err != nil {
	// 		return err
	// 	}
	// }

	err := root.ValidateGlobals(cmd, args)
	if err != nil {
		return err
	}

	return nil
}
