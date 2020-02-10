# Stable Matching Library

## Summary
This is the library for simulations in our research project on stable matchings in tiered markets.

This codebase offers executables to run experiments using the deferred acceptance (DA) algorithm, including:
* `./main`: used for light weight experiments, mostly for development and testing purpose.
* `./growingMarketLin` and `./growingMarketLog`: experiments where we fix the tier configuration of the market and increase the size of the market, balanced or unbalanced. The growth of the market size is linear or exponential in the two cases (with the latter supposedly better named `growingMarketExp`, but the current version reflects that the plot will be in log scale). We record the market configurations and average ranks into files prefixed `fixed_tiers_growing_market` or `fixed_tiers_growing_market_lin`.
* `./varyingTier`: fixes market size and varies the tier configuration. Similarly records market configurations and average ranks into files prefixed `fixed_market_varying_{tier_sizes|scores}`.
* `./uniquePartnerCount`: varies the balance of the market and compute the number of unique partners (e.g. identical under MOSM and WOSM). Writes to `unique_partner_count` files.
* `./imbalancedMarket`: fixes tier configurations and tier size of one side (proposing side), and makes the other side vary. For simplicity, the proposal-receiving side is homogeneous (i.e. only one tier). Records average ranks to `imbalanced_market` files.
* `./distributionOfPairs`: considers the macro distribution of matched pairs across tiers on the two sides. For convenience, we set two tiers of equal size on each side. The fraction of tier-1-to-tier-1 matches is recorded in `distribution_of_pairs_{grow_market|vary_scores}` files.

Aside from the experiments, we offer a Python program `analysisMatchingExperiments.py` to analyze and plot the data. The plots are collected in the `Figures/` directory.

## Implementation
The library implements two basic classes: `Matching` for setting up a market and performing DA (or its variant), and `Agent` for handling agent specific micro actions such as proposing, rejecting, and simulating total rank for an agent.

## Contributors
* Itai Ashlagi, Mark Braverman, Clayton Thomas, Geng Zhao (initial coding work)
