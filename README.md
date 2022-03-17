TRUST4
=======

Described in: 

Song, L., Cohen, D., Ouyang, Z. et al. TRUST4: immune repertoire reconstruction from bulk and single-cell RNA-seq data. Nat Methods (2021). https://doi.org/10.1038/s41592-021-01142-2

	Copyright (C) 2018- and GNU GPL by Li Song, Shirley Liu

Includes portions copyright from: 

	samtools - Copyright (C) 2008-, Genome Research Ltd, Heng Li
	

### What is TRUST4?

Tcr Receptor Utilities for Solid Tissue (TRUST) is a computational tool to analyze TCR and BCR sequences using unselected RNA sequencing data, profiled from solid tissues, including tumors. TRUST4 performs de novo assembly on V, J, C genes including the hypervariable complementarity-determining region 3 (CDR3) and reports consensus of BCR/TCR sequences. TRUST4 then realigns the contigs to IMGT reference gene sequences to report the corresponding information. TRUST4 supports both single-end and paired-end sequencing data with any read length. 

### Install

1. Clone the [GitHub repo](https://github.com/liulab-dfci/TRUST4), e.g. with `git clone https://github.com/liulab-dfci/TRUST4.git`
2. Run `make` in the repo directory

You will find the executable files in the downloaded directory. If you want to run TRUST4 without specifying the directory, you can either add the directory of TRUST4 to the environment variable PATH or create a soft link ("ln -s") of the file "run-trust4" to a directory in PATH.

TRUST4 depends on [pthreads](http://en.wikipedia.org/wiki/POSIX_Threads) and samtools depends on [zlib](http://en.wikipedia.org/wiki/Zlib). For MacOS, TRUST4 has been successfully compiled with gcc_darwin17.7.0 and gcc_9.2.0 installed by Homebrew.

TRUST4 is also available form [Bioconda](https://anaconda.org/bioconda/trust4). You can install TRUST4 with `conda install -c bioconda trust4`

### Usage

	Usage: ./run-trust4 [OPTIONS]
		Required:
			-b STRING: path to bam file
			-1 STRING -2 STRING: path to paired-end read files
			-u STRING: path to single-end read file
			-f STRING: path to the fasta file coordinate and sequence of V/D/J/C genes
		Optional:
			--ref STRING: path to detailed V/D/J/C gene reference file, such as from IMGT database. (default: not used). (recommended) 
			-o STRING: prefix of output files. (default: inferred from file prefix)
			--od STRING: the directory for output files. (default: ./)
			-t INT: number of threads (default: 1)
			--barcode STRING: if -b, bam field for barcode; if -1 -2/-u, file containing barcodes (defaul: not used)
			--barcodeRange INT INT CHAR: start, end(-1 for lenght-1), strand in a barcode is the true barcode (default: 0 -1 +)
			--barcodeWhitelist STRING: path to the barcode whitelist (default: not used)
			--read1Range INT INT: start, end(-1 for length-1) in -1/-u files for genomic sequence (default: 0 -1)
			--read2Range INT INT: start, end(-1 for length-1) in -2 files for genomic sequence (default: 0 -1)
			--UMI STRING: if -b, bam field for UMI; if -1 -2/-u, file containing UMIs (default: not used)
			--umiRange INT INT CHAR: start, end(-1 for lenght-1), strand in a UMI is the true UMI (default: 0 -1 +)
			--mateIdSuffixLen INT: the suffix length in read id for mate. (default: not used)
			--skipMateExtension: do not extend assemblies with mate information, useful for SMART-seq (default: not used)
			--abnormalUnmapFlag: the flag in BAM for the unmapped read-pair is nonconcordant (default: not set)
			--noExtraction: directly use the files from provided -1 -2/-u to assemble (default: extraction first)
			--repseq: the data is from TCR-seq or BCR-seq (default: not set)
			--outputReadAssignment: output read assignment results to the prefix_assign.out file (default: no output)
			--stage INT: start TRUST4 on specified stage (default: 0)
				0: start from beginning (candidate read extraction)
				1: start from assembly
				2: start from annotation
				3: start from generating the report table

### Input/Output

The primary input to TURST4 is the alignment of RNA-seq reads in BAM format(-b), the file containing the genomic sequence and coordinate of V,J,C genes(-f), and the reference database sequence containing annotation information, such as IMGT (--ref).

An alternative input to TRUST4 is the raw RNA-seq files in fasta/fastq format (-1/-2 for paired; -u for single-end). You still need the files like -f, --ref from above. In this case, you can directly use IMGT's seuqence file for -f. 

TRUST4 outputs several files. trust_raw.out, trust_final.out are the contigs and corresponding nucleotide weight. trust_annot.fa is in fasta format for the annotation of the consensus assembly. trust_cdr3.out reports the CDR1,2,3 and gene information for each consensus assemblies. And trust_report.tsv is a report file focusing on CDR3 and is compatible with other repertoire analysis tool such as VDJTools. 

Each header of trust_annot.fa is split into fields:

	consensus_id consensus_length average_coverage annotations

"annotations" also has several field, corresponding to annotation of V,D,J,C, CDR1, CDR2 and CDR3 respectively. For the annotation of the genes, it follows the pattern 

	gene_name(reference_gene_length):(consensus_start-consensus_end):(reference_start-reference_length):similarity
	
Each type of genes has at most three gene candidate ranked by their similarity. For the annotation of CDRs, it follows the pattern:

	CDRx(consensus_start-consensus_end):score=sequence
	
For CDR1,2, score is similarity. for CDR3, score 0.00 means partial CDR3, score 1.00 means CDR3 with imputed nucleotides and other numbers means the motif signal strength with 100.00 as strongest.

The coordinate is 0-based.

The output trust_cdr3.out is a tsv file. The fields are:

	consensus_id	index_within_consensus	V_gene	D_gene	J_gene	C_gene	CDR1	CDR2	CDR3	CDR3_score	read_fragment_count CDR3_germline_similarity full_length_assembly
	
Please note that CDR3_score in trust_cdr3.out has been divided by 100, so 1.00 is the maximum score and 0.01 means imputed CDR3.

The output trust_report.tsv is a tsv file. The fileds are:
	
	read_count	frequency(proportion of read_count)	CDR3_dna	CDR3_amino_acids	V	D	J	C	consensus_id consensus_id_full_length

For frequency, the BCR(IG) and TCR(TR) chains are normalized respectively. 

The output trust_airr.tsv follows [the AIRR format](https://docs.airr-community.org/en/latest/datarep/rearrangements.html). 

### Practical notes

* #### Existing V,J,C gene database (files for --ref and -f) for human and mouse

For users' conveninece, we have generated database files for human and mouse in the Git repo. 

For human, please use `hg38_bcrtcr.fa` (Gencode v35) for `-f` and use `human_IMGT+C.fa` for `--ref`. We also provide `hg19_bcrtcr.fa` for `-f` for users who want to use human genome GRCh37/hg19.

For mouse, please use `mouse/GRCm38_bcrtcr.fa` (Gencode vM22) for `-f` and use `mouse/mouse_IMGT+C.fa` for `--ref`.

* #### Build custom V,J,C gene database (files for --ref and -f)

Normally, the file specified by "--ref" is downloaded from IMGT website, For example, for human, you can use command

	perl BuildImgtAnnot.pl Homo_sapien > IMGT+C.fa

The available species name can be found on [IMGT FTP](http://www.imgt.org//download/V-QUEST/IMGT_V-QUEST_reference_directory/).

To generate the file specified by "-f", you need the reference genome of the species you are interested in and corresponding genome annotation GTF file. Then you can use command 
	
	perl BuildDatabaseFa.pl reference.fa annotation.gtf bcr_tcr_gene_name.txt > bcrtcr.fa

to generate the input for "-f". The "bcr_tcr_gene_name.txt" can be generated from `IMGT+C.fa` file using

	grep ">" IMGT+C.fa | cut -f2 -d'>' | cut -f1 -d' ' | cut -f1 -d'*' | sort | uniq > bcr_tcr_gene_name.txt

For users' convenience, we provide generated "bcr_tcr_gene_name.txt" for human and mouse as `human_vjc_IMGT.list` and `mouse/mouse_vdj_IMGT.list` respectively.

* #### 10X Genomics data:

When given barcode, TRUST4 only assembles the reads with the same barcode together. For 10X Genomics data, usually the input is the BAM file from cell-ranger, and you can use "--barcode" to specify the field in the BAM file to specify the barcode: e.g. "--barcode CB".

If your input is raw FASTQ files, you can use "--barcode" to specify the barcode file and use "--barcodeRange" to tell TRUST4 how to extract barcode information. If the barcode or UMI sequence is in the read sequence, you may use "--read1Range", "--read2Range" to tell TRUST4 how to extract sequence information in the reads. TRUST4 supports using wildcard in the -1 -2/-u option, so a typical way to run 10X Genomics single-end data is by:

	run-trust4 -f hg38_bcrtcr.fa --ref human_IMGT+C.fa -u path_to_10X_fastqs/*_R2_*.fastq.gz --barcode path_to_10X_fastqs/*_R1_*.fastq.gz --barcodeRange 0 15 + --barcodeWhitelist cellranger_folder/cellranger-cs/VERSION/lib/python/cellranger/barcodes/737K-august-2016.txt [other options]

The exact options depend on your 10X Genomics kit.

In the output, the abundance in the report will use the number of barcodes for the CDR3 instead of read count. TRUST4 will also generate the file trust_barcode_report.tsv. In this file, TRUST4 will pick the most abundance pair of chains as the representative for the barcode(cell). The format is:

	barcode	cell_type	IGH/TRB/TRD_information	IGK/IGL/TRA/TRG_information	secondary_chain1_information	secondary_chain2_information

For the chain information it is in CSV format:
	
	V_gene,D_gene,J_gene,C_gene,cdr3_nt,cdr3_aa,read_cnt,consensus_id,CDR3_germline_similarity,consensus_full_length

TRUST4 also converts the barcode report file to the trust_barcode_airr.tsv file to follow the AIRR format.

* #### SMART-Seq data

We provide a wrapper "trust-smartseq.pl" to process the files from platforms like SMART-seq. The user shall give the . An example command can be

	perl trust-smartseq.pl -1 read1_list.txt -2 read2_list.txt -t 8 -f hg38_bctcr.fa --ref human_IMGT+C.fa -o TRUST

The script will create two files: TRUST_report.tsv for general summary and TRUST_annot.fa for assemblies. The formats are described above. Each cell's name is inferred by the file name before the first ".".

* #### Simple report

The last step of generating simple report can be done with the command:

	perl trust-simplerep.pl trust_cdr3.out > trust_report.out

If you are interested in a subset of chains, you can "grep" those from trust_cdr3.out and run trust-simplerep.pl on the subset.
 
### Example

The directory './example' in this distribution contains one BAM files as input for TRUST4. Run TRUST4 with:

	./run-trust4 -b example/example.bam -f hg38_bcrtcr.fa --ref human_IMGT+C.fa

The run will generate the files TRUST_example_raw.out, TRUST_example_final.out, TRUST_example_annot.fa, TRUST_example_cdr3.out, TRUST_example_report.tsv and several fq/fa files in seconds. The results should be the same as the files in the example folder.

The directory also contains two fastq files, and you can run TRUST4 with:

	./run-trust4 -f human_IMGT+C.fa --ref human_IMGT+C.fa -1 example/example_1.fq -2 example/example_2.fq -o TRUST_example

The run will generate the files mentioned above from BAM input. Your results should be like the files example_annot.fa, 

### Miscellaneous

The evaluation instructions and scripts in TRUST4's manuscript is available at: https://github.com/liulab-dfci/TRUST4_manuscript_evaluation .

### Terms of use

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received (LICENSE.txt) a copy of the GNU General
Public License along with this program; if not, you can obtain one from
http://www.gnu.org/licenses/gpl.txt or by writing to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
### Support

Create a [GitHub issue](https://github.com/liulab-dfci/TRUST4/issues).
