//---------------------------------------------------------
// Copyright 2015 Ontario Institute for Cancer Research
// Written by Jared Simpson (jared.simpson@oicr.on.ca)
//---------------------------------------------------------
//
// nanopolish_poremodel -- Representation of the Oxford
// Nanopore sequencing model, as described in a FAST5 file
//
#ifndef NANOPOLISH_POREMODEL_H
#define NANOPOLISH_POREMODEL_H

#include <assert.h>
#include "nanopolish_common.h"
#include <inttypes.h>
#include <string>
#include <map>
#include "../fast5/src/fast5.hpp"

//
struct PoreModelStateParams
{
    double level_mean;
    double level_stdv;
    double sd_mean;
    double sd_stdv;

    double level_log_stdv;
    double sd_lambda;
    double sd_log_lambda;

    PoreModelStateParams& operator = (const fast5::Model_Entry& e)
    {
        level_mean = e.level_mean;
        level_stdv = e.level_stdv;
        sd_mean = e.sd_mean;
        sd_stdv = e.sd_stdv;
        update_sd_lambda();
        return *this;
    }

    // update sd_lambda based on sd_mean & sd_stdv
    void update_sd_lambda()
    {
        sd_lambda = pow(sd_mean, 3.0) / pow(sd_stdv, 2.0);
    }
    // update sd_stdv based on sd_mean & sd_lambda
    void update_sd_stdv()
    {
        sd_stdv = pow(pow(sd_mean, 3.0) / sd_lambda, .5);
    }
    void update_logs()
    {
        level_log_stdv = log(level_stdv);
        sd_log_lambda = log(sd_lambda);
    }
};

//
class PoreModel
{
    public:
        PoreModel(uint32_t _k=5) : is_scaled(false), k(_k), pmalphabet(&gDNAAlphabet) {}

        // These constructors and the output routine take an alphabet 
        // so that kmers are inserted/written in order
        // nicer might be to store the states as a map from kmer -> state

        PoreModel(const std::string filename, const Alphabet *alphabet=NULL);
        PoreModel(fast5::File *f_p, const size_t strand, const Alphabet *alphabet=NULL);

        void write(const std::string filename, const std::string modelname="");

        inline GaussianParameters get_scaled_parameters(const uint32_t kmer_rank) const
        {
            assert(is_scaled);
            return scaled_params[kmer_rank];
        }

        inline PoreModelStateParams get_scaled_state(const uint32_t kmer_rank) const
        {
            assert(is_scaled);
            return scaled_states[kmer_rank];
        }

        inline PoreModelStateParams get_parameters(const uint32_t kmer_rank) const
        {
            return states[kmer_rank];
        }
        
        inline size_t get_num_states() const { return states.size(); }

        // Pre-compute the GaussianParameters to avoid
        // taking numerous logs in the emission calculations
        void bake_gaussian_parameters();

        // update states with those given, or from another model
        void update_states( const PoreModel &other );
        void update_states( const std::vector<PoreModelStateParams> &otherstates );

        //
        // Data
        //

        // model metadata
        std::string model_filename;
        std::string name;
        uint32_t k;

        // per-read scaling parameters
        double scale;
        double shift;
        double drift;
        double var;
        double scale_sd;
        double var_sd;

        // to support swapping models, a .model file might contain a shift_offset field
        // which describes how to change the per-read shift values to match the incoming
        // model. This field stores this data, which might be 0.
        double shift_offset;

        bool is_scaled;

        const Alphabet *pmalphabet; 

        std::vector<PoreModelStateParams> states;
        std::vector<PoreModelStateParams> scaled_states;
        std::vector<GaussianParameters> scaled_params;
};

typedef std::map<std::string, PoreModel> ModelMap;

ModelMap read_models_fofn(const std::string& fofn_name, const Alphabet *alphabet=nullptr);

#endif
