c .. arrays holding index of closest 'town' in each group 
c 13-MAR-1996 DDG - Aligned for Alpha
c 28-APR-1998 DDG - Added town list structure

c ... Stores root path name for town and fault files

      parameter       (MAX_GRPS = 8)
      
      type where_structure
         integer       ngrps       !count of groups
         integer       ipad        !alignment
         character*80  cwhere(MAX_GRPS) !where strings
         character*80  cfault   !fault string
      end type
      
      type(where_structure) where_rec
      common/where_structure/ where_rec
      
      parameter       (MAX_TOWNS = 2000)
      
      type town_list
         integer*4     ntowns
         character*50  name(MAX_TOWNS)
         real*4        slat(MAX_TOWNS)
         real*4        slon(MAX_TOWNS)
         integer*4     ngroup(MAX_TOWNS)
      end type
      
      type(town_list)       town
      common/town_list/       town
      
      character*79    cwhere
      character*79    cfault

c ... array of strings holding location descriptions

      common/where_out/cwhere(max_grps)
      common/fault_out/cfault

c ... paths to the town and fault file

      type root_path
         character*80 townfile
         character*80 faultfile
      end type
      
      type(root_path)       paths
      common/root_path/       paths

c Use where.prevous_lat and where.previous_lon to avoid repeating the where 
c  look-up work on multiple calls to WHERE, as when constructing many different 
c  formats for the same event
