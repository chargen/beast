<!-- GNU LGPL v2.1+: http://www.gnu.org/licenses/lgpl.html -->

<docs>
  ## vc-projectshell - Shell for editing and display of a Bse.Project.
  ### Props:
  - **project** - Implicit, using App.bse_project().
</docs>

<template>
  <div class="vc-projectshell">
    <div class="vc-projectshell-track-area">
      Project Shell:
      <vc-playcontrols :project="project"> </vc-playcontrols>
      <vc-track-list :song="song"></vc-track-list>
    </div>
    <div class="vc-projectshell-part-area" style="display: flex; overflow: hidden;" >
      <vc-piano-roll :piano-part="this.piano_roll_part" ></vc-piano-roll>
    </div>
  </div>
</template>

<style lang="scss">
  @import 'mixins.scss';
  .vc-projectshell { }
  .vc-projectshell-part-area {
    background-color: $vc-button-border;
    height: 350px; }
</style>

<script>
module.exports = {
  name: 'vc-projectshell',
  data_tmpl: {
    piano_roll_part: undefined,
    show_about_dialog: false,
  },
  computed: {
    project: function () {
      if (!this.project_) {
	this.project_ = Bse.server.create_project ('Untitled');
	if (this.project_) {
	  let example = __dirname + "/../../../" + 'Demos/partymonster.bse';
	  this.project_.restore_from_file (example);
	}
      }
      return this.project_;
    },
    song: function () {
      let s, supers = this.project.get_supers();
      for (s of supers) {
	if (s instanceof Bse.Song)
	  return s;
      }
      return undefined;
    },
  },
  created() {
    // delete `Shell` dummy on toplebel if present
    let p = this;
    while (p.$parent)
      p = p.$parent;
    if (p.Shell && p.Shell.Shell_placeholder)
      delete p.Shell;
    // inject Shell into all Vue components
    Vue.prototype.Shell = this;
    assert (this === p.Shell);
    // force root updates with new Shell properties in place
    (p || this).$forceUpdate();
  },
  provide () { return { 'vc-projectshell': this }; },
  methods: {
    open_part_edit (part) {
      assert (part instanceof Bse.Part);
      this.piano_roll_part = part;
    },
    status (...args) {
      console.log (...args);
    },
  },
};
</script>
