/* 
 * File:   bidder_selector.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 16 февраля 2017 г., 21:15
 */

#ifndef BIDDER_SELECTOR_HPP
#define BIDDER_SELECTOR_HPP

#include "core/openrtb.hpp"
#include "bidder_caches.hpp"
#include <memory>

namespace vanilla {
template<typename Config = BidderConfig>
class BidderSelector {
    public:
        using SpecBidderCaches = BidderCaches<Config>;
       
        struct intersc_cmp {
            bool operator()(const uint64_t &l, const Ad &r) const {
                return l < r.ad_id;
            }
            bool operator()(const Ad &l, const uint64_t &r) const {
                return l.ad_id < r;
            }
        };
        BidderSelector(BidderCaches<Config> &bidder_caches):
            bidder_caches{bidder_caches}
        {
        }
        template<typename T> 
        std::shared_ptr<Ad> select(const openrtb::BidRequest<T> &req, const openrtb::Impression<T> &imp) {
            std::shared_ptr <Ad> result;
            
            Geo geo;
            if(!getGeo(req, geo) ) {
                LOG(debug) << "No geo";
                return result;
            }
            
            if(!getGeoCampaigns(geo.geo_id)) {
                LOG(debug) << "No campaigns for geo " << geo.geo_id;
                return result;
            }
            else {
                LOG(debug) << "Selected campaigns " << geo_campaigns.size();
            }
            
            if(!getCampaignAds(geo_campaigns)) {
                LOG(debug) << "No ads for geo " << geo.geo_id;
                return result;
            }
            else {
                LOG(debug) << "selected ads " << campaign_ads.size();
            }
            /*********** TODO: decommision Ad cache and transfer ad.w ad.h into campaign_data
            retrieved_cached_ads.clear();
            if(!bidder_caches.ad_data_entity.retrieve(retrieved_cached_ads, imp.banner.get().w, imp.banner.get().h)) {
                return result;
            }
            LOG(debug) << "size (" << imp.banner.get().w << "x"  << imp.banner.get().h << ") ads count " << retrieved_cached_ads.size();
            
            intersection.clear();
            std::set_intersection(retrieved_cached_ads.begin(), retrieved_cached_ads.end(),
                campaign_ads.begin(), campaign_ads.end(),
                std::back_inserter(intersection),
                intersc_cmp()
                //[](auto lhs, auto rhs) { return lhs->ad_id < rhs->ad_id; }
                );
            // Sort by cicros
            // TODO make ability to make custom algorithm
            std::sort(intersection.begin(), intersection.end(),
                [](const Ad &first, const Ad &second) -> bool {
                return first.max_bid_micros > second.max_bid_micros;
            });
            LOG(debug) << "intersecion size " << intersection.size();
            if(intersection.size()) {
                result = std::make_shared<Ad>(intersection[0]);
            }
            return result;
            *************************************************************/
            return result;
        }
        
        bool getGeoCampaigns(uint32_t geo_id) {
            geo_campaigns.clear();
            if (!bidder_caches.geo_campaign_entity.retrieve(geo_campaigns, geo_id)) {
                LOG(debug) << "GeoAd retrieve failed " << geo_id;
                return false;
            }
            return true;
        }
        template <typename T>
        bool getGeo(const openrtb::BidRequest<T> &req, Geo &geo) {
            if (!req.user) {
                LOG(debug) << "No user";
                return true;
            }
            if (!req.user.get().geo) {
                LOG(debug) << "No user geo";
                return true;
            }
           
            auto &city_view = req.user.get().geo.get().city ;
            auto &country_view = req.user.get().geo.get().country;
            const std::string city = boost::algorithm::to_lower_copy(std::string(city_view.data(), city_view.size()));
            const std::string country = boost::algorithm::to_lower_copy(std::string(country_view.data(), country_view.size()));

            if (!bidder_caches.geo_data_entity.retrieve(geo, city, country)) {
                LOG(debug) << "retrieve failed " << city << " " << country;
                return false;
            }
            
            return true;
        }
        bool getCampaignAds(const std::vector<GeoCampaign> &campaigns) {
           campaign_ads.clear();
           campaign_data.clear();
           LOG(debug) << "select ads from  " << campaigns.size() << " campaigns";
           for (auto &campaign : campaigns) {
                LOG(debug) << "search campaign " << campaign;
                campaign_data.clear();
                if (!bidder_caches.campaign_data_entity.retrieve(campaign_data, campaign.campaign_id)) {
                    continue;
                }
                LOG(debug) << "ads in campaign " << campaign_data.size();
             
                std::for_each(std::begin(campaign_data), std::end(campaign_data), [&](auto &cd) {
                    campaign_ads.insert(cd.ad_id);
                });
           }
           return campaign_ads.size() > 0;
        }
    private:   
        SpecBidderCaches &bidder_caches;
        std::vector<GeoCampaign> geo_campaigns;
        std::vector<CampaignData> campaign_data;
        std::vector<Ad> retrieved_cached_ads;
        std::set<uint64_t> campaign_ads;
        std::vector<Ad> intersection;
};
}

#endif /* BIDDER_SELECTOR_HPP */

