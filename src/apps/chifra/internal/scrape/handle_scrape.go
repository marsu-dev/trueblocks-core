package scrapePkg

// Copyright 2021 The TrueBlocks Authors. All rights reserved.
// Use of this source code is governed by a license that can
// be found in the LICENSE file.

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"

	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/colors"
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/config"
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/index"
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/logger"
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/rpcClient"
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/tslib"
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/utils"
	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/validate"
)

// TODO: We should repond to non-tracing (i.e. Geth) nodes better
// TODO: Make sure we're not running acctScrape and/or pause if it's running
func (opts *ScrapeOptions) HandleScrape() error {
	progress, err := rpcClient.GetMetaData(opts.Globals.Chain, opts.Globals.TestMode)
	if err != nil {
		logger.Log(logger.Error, err)
		return err
	}

	blazeOpts := BlazeOptions{
		Chain:         opts.Globals.Chain,
		NChannels:     opts.Settings.Channel_count,
		NProcessed:    0,
		StartBlock:    opts.StartBlock,
		BlockCount:    opts.BlockCnt,
		UnripeDist:    opts.Settings.Unripe_dist,
		RpcProvider:   config.GetRpcProvider(opts.Globals.Chain),
		AppearanceMap: make(index.AddressAppearanceMap, opts.Settings.Apps_per_chunk),
		TsArray:       make([]tslib.Timestamp, 0, opts.BlockCnt),
		ProcessedMap:  make(map[int]bool, opts.BlockCnt),
	}

	if ok, err := opts.HandlePrepare(progress, &blazeOpts); !ok || err != nil {
		logger.Log(logger.Error, err)
		return err
	}

	origBlockCnt := opts.BlockCnt
	for {
		progress, err = rpcClient.GetMetaData(opts.Globals.Chain, opts.Globals.TestMode)
		if err != nil {
			logger.Log(logger.Error, err)
			break
		}

		<-opts.Pause(progress)

		// We start the current round one block past the end of the previous round
		opts.StartBlock = utils.Max(progress.Ripe, utils.Max(progress.Staging, progress.Finalized)) + 1
		// And each round we assume we're going to process this many blocks...
		opts.BlockCnt = origBlockCnt
		if (opts.StartBlock + opts.BlockCnt) > progress.Latest {
			// ...unless we're too close to the head, then we shorten the number of blocks to process
			opts.BlockCnt = (progress.Latest - opts.StartBlock)
		}

		// The 'ripeBlock' is the head of the chain unless the chain is further along
		// than 'UnripeDist.' If it is, the `ripeBlock` is 'UnripeDist' behind the
		// head (i.e., 28 blocks usually - six minutes)
		ripeBlock := progress.Latest
		if ripeBlock > opts.Settings.Unripe_dist {
			ripeBlock = progress.Latest - opts.Settings.Unripe_dist
		}

		blazeOpts = BlazeOptions{
			Chain:         opts.Globals.Chain,
			NChannels:     opts.Settings.Channel_count,
			NProcessed:    0,
			StartBlock:    opts.StartBlock,
			BlockCount:    opts.BlockCnt,
			RipeBlock:     ripeBlock,
			UnripeDist:    opts.Settings.Unripe_dist,
			RpcProvider:   config.GetRpcProvider(opts.Globals.Chain),
			AppearanceMap: make(index.AddressAppearanceMap, opts.Settings.Apps_per_chunk),
			TsArray:       make([]tslib.Timestamp, 0, origBlockCnt),
			ProcessedMap:  make(map[int]bool, origBlockCnt),
		}

		// Remove whatever's in the unripePath before running each round. We do this
		// because the chain may have re-organized (which it does frequently). This is
		// why we have an unripePath.
		unripePath := filepath.Join(config.GetPathToIndex(opts.Globals.Chain), "unripe")
		err = os.RemoveAll(unripePath)
		if err != nil {
			logger.Log(logger.Error, err)
			break
		}

		// In some cases, the index may be already ahead of the chain tip. (For example,
		// we may be dealing with a node installation that is being re-synced, but the
		// index already exists.) In this case, we sleep for a while to allow the chain
		// to catch up.
		m := utils.Max(progress.Ripe, utils.Max(progress.Staging, progress.Finalized)) + 1
		if m > progress.Latest {
			fmt.Println(validate.Usage("The index ({0}) is ahead of the chain ({1}).", fmt.Sprintf("%d", m), fmt.Sprintf("%d", progress.Latest)))
			continue
		}

		// Here we do the actual scrape for this round. If anything goes wrong, the
		// function will have cleaned up (i.e. remove the unstaged ripe blocks). Note
		// that we don't quit, instead we sleep and we retry continually.
		if err := opts.HandleScrapeBlaze(progress, &blazeOpts); err != nil {
			logger.Log(logger.Error, colors.BrightRed, err, colors.Off)
			continue
		}
		blazeOpts.syncedReporting(int(blazeOpts.StartBlock+blazeOpts.BlockCount), true /* force */)

		if ok, err := opts.HandleScrapeConsolidate(progress, &blazeOpts); !ok || err != nil {
			logger.Log(logger.Error, err)
			if !ok {
				break
			}
			continue
		}
	}

	// should never append (see caller)
	return errors.New("unexpected loop exit")
}