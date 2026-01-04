import { Fragment } from '../../../../neonjsx/runtime';
import { isEnrolled } from '../../scripts/auth_state';
import { sse_cpu } from '../../scripts/sse/cpu';
import { sse_events } from '../../scripts/sse/events';
import { sse_memory } from '../../scripts/sse/memory';
import { sse_redirect_service } from '../../scripts/sse/redirect_service';
import { Footer } from '../footer';
import { BenchmarkToggle, initBenchmarkToggle } from './benchmark_toggle';
import { initCodexForm } from './codex';
import { Enrollment } from './enrollment';
import { Hero } from './hero';
import { initHistoryPanel } from './history_panel';
import { Stats } from './stats';
import { Timeline } from './timeline';
import { UserCheck } from './user_check';
import { UserRegister } from './user_register';
import { VisualToggle, initVisualToggle } from './visual_toggle';

type AppProps = { authenticated: boolean };

function sse(){
  sse_memory();
  sse_redirect_service();
  sse_cpu();
  sse_events();
}

export const AuthApp = ( { authenticated }: AppProps ) => {
  const enrolled = isEnrolled();

  queueMicrotask( () => {
    initBenchmarkToggle();
    initCodexForm();
    initHistoryPanel();
    initVisualToggle();
    sse();
  } );

  const handleLoginSuccess = () => {
    window.location.reload();
  };

  return (
    <div className="page admin">
      <Hero authenticated={authenticated} onLoginSuccess={handleLoginSuccess}/>
      {! authenticated ? (
        <Fragment>
          {! enrolled && <UserRegister/>}
          {! enrolled && <Enrollment onSuccess={handleLoginSuccess}/>}
        </Fragment>
      ) : (
        <Fragment>
          <div className="panel-grid">
            <BenchmarkToggle/>
            <VisualToggle/>
          </div>
          <UserCheck/>
        </Fragment>
      )}
      <Stats authenticated={authenticated}/>
      <Timeline/>
      <Footer/>
    </div>
  );
};
