<!-- GNU LGPL v2.1+: http://www.gnu.org/licenses/lgpl.html -->

<docs>
  ## vc-track-view - Vue template to display a song track
  ### Props:
  - **project** - The BSE project containing playback tracks.
  - **track** - The BseTrack to display.
</docs>

<template>

  <div class="vc-track-view" >
    <span class="vc-track-view-label">{{ track.get_name() }}</span>
    <span class="vc-track-view-partlist" >
      <vc-part-thumb v-for="(tp, pindex) in this.track.list_parts()" :key="tp.part.unique_id() + '-' + tp.tick"
		     :part="tp.part" :tick="tp.tick" :track-index="index" :index="pindex" ></vc-part-thumb>
    </span>
  </div>

</template>

<style lang="scss">
  @import 'mixins.scss';
  .vc-track-view {
    display: flex; align-items: center; /* vertical centering */
    background-color: $vc-button-border;
    border: 1px solid $vc-button-border;
    border-radius: $vc-button-radius; }
  .vc-track-view > *	{ margin: 0; }
  .vc-track-view-label {
    display: inline-block; width: 7em;
    text-overflow: ellipsis;
    overflow: hidden; white-space: nowrap;
  }
  .vc-track-view-partlist {
    display: inline-block;
    position: relative;
    height: $vc-track-list-row-height;	/* fixed height is required to accurately calculate vertical scroll area */
  }
</style>

<script>
module.exports = {
  name: 'vc-track-view',
  props: {
    'track': { type: Bse.Track, },
    'index': { type: Number, },
  },
  data_tmpl: {
    name: "Track-Label2",
  },
};
</script>
