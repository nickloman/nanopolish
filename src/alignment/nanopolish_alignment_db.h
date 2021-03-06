//---------------------------------------------------------
// Copyright 2015 Ontario Institute for Cancer Research
// Written by Jared Simpson (jared.simpson@oicr.on.ca)
//---------------------------------------------------------
//
// nanopolish_alignment_db -- abstraction for working
// with sets of reads/events aligned to a reference genome
//
#ifndef ALIGNMENT_DB
#define ALIGNMENT_DB

#include <string>
#include <vector>
#include <map>
#include "nanopolish_anchor.h"
#include "nanopolish_variant.h"

// structs
struct SequenceAlignmentRecord
{
    std::string sequence;
    std::vector<AlignedPair> aligned_bases;
};

struct EventAlignmentRecord
{
    SquiggleRead* sr;
    uint8_t rc; // with respect to reference genome
    uint8_t strand; // 0 = template, 1 = complement
    uint8_t stride; // whether event indices increase or decrease along the reference
    std::vector<AlignedPair> aligned_events;
};

// typedefs
typedef std::map<std::string, SquiggleRead*> SquiggleReadMap;

class AlignmentDB
{
    public:
        AlignmentDB(const std::string& reads_file,
                    const std::string& reference_file,
                    const std::string& sequence_bam,
                    const std::string& event_bam);

        ~AlignmentDB();

        void load_region(const std::string& contig,
                         int start_position,
                         int stop_position);
        
        const std::string& get_reference() const { return m_region_ref_sequence; }

        std::string get_reference_substring(const std::string& contig,
                                            int start_position,
                                            int stop_position) const;

        std::vector<std::string> get_read_substrings(const std::string& contig,
                                                     int start_position,
                                                     int stop_position) const;

        std::vector<HMMInputData> get_event_subsequences(const std::string& contig,
                                                         int start_position,
                                                         int stop_position) const;

        std::vector<HMMInputData> get_events_aligned_to(const std::string& contig, int position) const;

        std::vector<Variant> get_variants_in_region(const std::string& contig,
                                                    int start_position,
                                                    int stop_position,
                                                    double min_frequency,
                                                    int min_depth) const;

        int get_region_start() const { return m_region_start; }
        int get_region_end() const { return m_region_end; }
        
        void set_alternative_model(const ModelMap* p_model_map) { m_p_model_map = p_model_map; }

    private:
        
        void _load_sequence_by_region();
        void _load_events_by_region();
        void _clear_region();

        // Search the vector of AlignedPairs using lower_bound/upper_bound
        // and the input reference coordinates. If the search succeeds,
        // set read_start/read_stop to be the read_pos of the bounding elements
        // and return true. 
        bool _find_by_ref_bounds(const std::vector<AlignedPair>& pairs,
                                 int ref_start,
                                 int ref_stop,
                                 int& read_start,
                                 int& read_stop) const;

        bool _find_iter_by_ref_bounds(const std::vector<AlignedPair>& pairs,
                                      int ref_start,
                                      int ref_stop,
                                      AlignedPairConstIter& start_iter,
                                      AlignedPairConstIter& stop_iter) const;

        //
        // data
        //
        std::string m_reference_file;
        std::string m_sequence_bam;
        std::string m_event_bam;

        // loaded region
        std::string m_region_ref_sequence;
        std::string m_region_contig;
        int m_region_start;
        int m_region_end;

        // cached alignments for a region
        Fast5Map m_fast5_name_map;
        std::vector<SequenceAlignmentRecord> m_sequence_records;
        std::vector<EventAlignmentRecord> m_event_records;
        SquiggleReadMap m_squiggle_read_map;
        const ModelMap* m_p_model_map;
};

#endif
